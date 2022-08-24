
// C++
#include <new>
#include <map>
#include <utility>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <iomanip>
#include <variant>
#include <condition_variable>
// C
#include <stdio.h>
// Engine
#include "util/utils.h"
#include "entry_point/game_loop.h"
#include "net/net_message.h"
// Boost
#include <boost/noncopyable.hpp>

namespace {
    using byte_t = std::uint8_t;
}



namespace engine::memory
{
    /**
     * Класс-обёртка над сырой памятью(куча).
     */
    template <typename data_t>
    struct raw_memory_t
    {
        data_t * buf_     {nullptr};
        std::size_t size_ {0};

        static data_t* Allocate(std::size_t n) {
            return static_cast<data_t*>(operator new(n));
        }

        static void Deallocate(data_t* buf) {
            operator delete(buf);
        }

        // noncopyable
        raw_memory_t() = delete;
        raw_memory_t(const raw_memory_t<data_t>&) = delete;
        raw_memory_t<data_t> operator= (const raw_memory_t<data_t>&) = delete;

        explicit raw_memory_t(std::size_t n) {
            buf_ = Allocate(n);
            size_ = n;
            std::memset(buf_, 0x0, sizeof(data_t) * size_);
        }

        explicit raw_memory_t(raw_memory_t<data_t>&& other) {
            this->swap(other);
        }

        raw_memory_t<data_t> operator= (raw_memory_t<data_t>&& other) {
            this->swap(other);
        }

        ~raw_memory_t() {
            Deallocate(buf_);
            size_ = 0;
        }

        void swap(raw_memory_t& other) {
            std::swap(buf_, other.buf_);
            std::swap(size_, other.size_);
        }

        data_t* operator+ (std::size_t i) {
            return buf_ + i;
        }

        const data_t* operator+ (std::size_t i) const {
            return buf_ + i;
        }

        data_t& operator[] (std::size_t i) {
            return buf_[i];
        }

        const data_t& operator[] (std::size_t i) const {
            return buf_[i];
        }

        data_t* operator* (){
            return buf_;
        };
    }; // raw_memory
}

namespace engine::util
{
    inline void memcpy(void* dst, void* src, std::size_t n) noexcept {
        using ptr = byte_t*;
        auto dst_ = reinterpret_cast<ptr>(dst);
        auto src_ = reinterpret_cast<ptr>(src);
        for (std::size_t i = 0; i < n; ++i) { 
            dst_[i] = src_[i]; 
        }
    }

    inline void memmove(void* dst, void* src, std::size_t n) noexcept {
        using ptr = byte_t*;
        auto dst_ = reinterpret_cast<ptr>(dst);
        auto src_ = reinterpret_cast<ptr>(src);
        for (std::size_t i = 0; i < n; ++i) { 
            std::swap(dst_[i], src_[i]); 
            std::memset(src_, 0x0, 1);
        }
    }
} // engine::util

namespace engine::net::messaage_stock
{

} // engine::net::messaage_stock

namespace engine::net::queue
{
    /** Класс реализующий потоко-безопасную двойную очередь
     *  для обработки сообщений.
     */
    template <typename QueObject>
    class MessageQueue : boost::noncopyable
    {
    public:
        MessageQueue() = default;
        virtual ~MessageQueue() { clear(); }

    public:
        // Вернуть ссылку на первый элемент очереди
        const QueObject& front();
        // Вернуть ссылку на последний элемент очереди
        const QueObject& back();
        // Добавить элемент в начало очереди
        void push_front(QueObject&& item);
        // Добавить элемент в конец очереди
        void push_back(QueObject&& item);
        // Удалить первый элемент из очереди и вернуть его
        QueObject pop_front();
        // Удалить последний элемент из очереди и вернуть его
        QueObject pop_back();
        // Есть ли элементы в очереди?
        bool empty();
        // Вернуть количество элементов в очереди
        std::size_t count();
        // Очистить очередь
        void clear();

        // Ожидать поступление нового сообщения в очередь
        // (для оптимизации потока обработки сообщений)
        void wait();

    protected:
        // mutex (TODO: atomic)
        std::mutex mutex;
        // deque
        std::deque<QueObject> deq;
        // mutex для ожидающей блокировки поступающих сообщений (TODO: atomic)
        std::mutex wait_mutex;
        // cv сигнализации новых сообщений в очереди
        std::condition_variable cv_wait_block;

    };  // MessageQueue

    template <typename QueObject>
    const QueObject& engine::net::queue::MessageQueue<QueObject>::front()
    {
        std::scoped_lock lock(mutex);
        return deq.front();
    }

    template <typename QueObject>
    const QueObject& engine::net::queue::MessageQueue<QueObject>::back()
    {
        std::scoped_lock lock(mutex);
        return deq.back();
    }

    template <typename QueObject>
    void engine::net::queue::MessageQueue<QueObject>::push_front(QueObject&&  item)
    {
        {
            std::scoped_lock lock(mutex);
            deq.emplace_front(std::move(item));
        }

        // Новое сообщение поступило в очередь, сигнализируем об этом потоку обработки
        std::unique_lock<std::mutex> ilock(wait_mutex);
        cv_wait_block.notify_one();
    }

    template <typename QueObject>
    void engine::net::queue::MessageQueue<QueObject>::push_back(QueObject&&  item)
    {
        {
            std::scoped_lock lock(mutex);
            deq.emplace_back(std::move(item));
        }

        // Новое сообщение поступило в очередь, сигнализируем об этом потоку обработки
        std::unique_lock<std::mutex> ilock(wait_mutex);
        cv_wait_block.notify_one();
    }

    template <typename QueObject>
    QueObject engine::net::queue::MessageQueue<QueObject>::pop_front()
    {
        std::scoped_lock lock(mutex);
        auto item = std::move(deq.front());
        deq.pop_front();
        return item;
    }

    template <typename QueObject>
    QueObject engine::net::queue::MessageQueue<QueObject>::pop_back()
    {
        std::scoped_lock lock(mutex);
        auto item = std::move(deq.back());
        deq.pop_back();
        return item;
    }

    template <typename QueObject>
    bool engine::net::queue::MessageQueue<QueObject>::empty()
    {
        std::scoped_lock lock(mutex);
        return deq.empty();
    }

    template <typename QueObject>
    std::size_t engine::net::queue::MessageQueue<QueObject>::count()
    {
        std::scoped_lock lock(mutex);
        return deq.size();
    }

    template <typename QueObject>
    void engine::net::queue::MessageQueue<QueObject>::clear()
    {
        std::scoped_lock lock(mutex);
        deq.clear();
    }

    template <typename QueObject>
    void engine::net::queue::MessageQueue<QueObject>::wait()
    {
        while(this->empty())
        {
            std::unique_lock<std::mutex> ulock(wait_mutex);
            cv_wait_block.wait(ulock);
        }
    }

} // engine::net::queue

namespace engine::net::message
{
    static constexpr std::size_t DATA_LIMIT = 52;     // размер полезной нагрузки
    static constexpr std::size_t MESSAGE_SIZE = 64;   // размер сетевого пакета

    /**
     *  Тип сетевого сообщения. Отвечает за тип обработки
     *  сообщения и указывает способ, которым его нужно переслать.
     *  Размер - 4 байта
     */
    struct message_id
    {
        message_id() = default;
        // noncopyable
        message_id(const message_id&) = delete;
        message_id& operator= (const message_id&) = delete;

        message_id(message_id&& other) noexcept
            : action(action_t::NONE)
            , protocol(proto_t::TCP)
        {
            std::swap(action, other.action);
            std::swap(protocol, other.protocol);
        }

        message_id& operator= (message_id&& other) noexcept
        {
            if (this != &other)
            {
                std::swap(action, other.action);
                std::swap(protocol, other.protocol);
            }
            return *this;
        }

        // ID - тип сообщения
        enum class action_t : std::uint16_t
        {
            NONE = 0,
            PING,
            ACTION,
            CHAT_MESSAGE
        };

        // Rule - протокол которым будет передаваться сообщение
        enum class proto_t : std::uint16_t
        {
            TCP = 0,
            UDP
        };

        action_t    action;         // action id                    [2 byte]
        proto_t     protocol;       // message transmitted behavior [2 byte]

        friend std::ostream& operator<< (std::ostream& os, const message_id& id) {
            std::string s;

            s.append("[").append("action - ");
            switch(id.action) 
            {
                case action_t::PING:    s.append("PING");   break;
                case action_t::ACTION:  s.append("ACTION"); break;
                default: s.append("unknown");
            }

            s.append(", protocol - ").append(id.protocol == proto_t::TCP ? "TCP" : "UDP").append("]");
            return os << s;
        }
    }; // message_id

    /**
     *  Заголовок сетевого сообщения. Содержит тип отправляемого
     *  сообщения, код доступа, флаги кодирования и сжатия
     *  Размер - 10 байт
     */
    template <typename message_id_t>
    struct message_header_t
    {
        message_header_t() = default;
        // noncopyable
        message_header_t(const message_header_t<message_id_t>&) = delete;
        message_header_t<message_id_t>& operator= (const message_header_t<message_id_t>&) = delete;

        message_header_t(message_header_t<message_id_t>&& other) noexcept
        {
            std::swap(type, other.type);
            std::swap(uuid, other.uuid);
            std::swap(block_num, other.block_num);
            std::swap(block_count, other.block_count);
            std::swap(access_code, other.access_code);
            std::swap(encode, other.encode);
            std::swap(compress, other.compress);
        }

        message_header_t<message_id_t>& operator= (message_header_t<message_id_t>&& other) noexcept
        {
            if (this != &other)
            {
                std::swap(type, other.type);
                std::swap(uuid, other.uuid);
                std::swap(block_num, other.block_num);
                std::swap(block_count, other.block_count);
                std::swap(access_code, other.access_code);
                std::swap(encode, other.encode);
                std::swap(compress, other.compress);
            }
            return *this;
        }

        message_id_t    type        {};     // message type id              [4 byte]
        std::uint8_t    uuid        {0};    // unique message id            [1 byte]
        std::uint8_t    block_num   {1};    // message block number         [1 byte]
        std::uint8_t    block_count {1};    // message block count          [1 byte]
        std::uint8_t    access_code {0};    // endpoint session access code [1 byte]
        bool            encode      {false};// message encode flag          [1 byte]
        bool            compress    {false};// message compress flag        [1 byte]

        friend std::ostream& operator << (std::ostream& os, const message_header_t<message_id_t>& header) 
        {
            os << "header:" 
                << "\n\ttype        - " << header.type
                << "\n\tuuid        - " << header.uuid
                << "\n\tblock_num   - " << header.block_num
                << "\n\tblock_count - " << header.block_count
                << "\n\taccess_code - " << header.access_code 
                << "\n\tencode      - " << (header.encode ? "true" : "false") 
                << "\n\tcompress    - " << (header.compress ? "true" : "false")
                << "\n";
            return os;
        }
    }; // message_header_t

    /**
     *  Тело сетевого сообщения. Обёртка над std::array 
     *  с операциями чтения и записи в буфер сообщения.
     *  Размер - 54 байт
     */
    struct message_body_t
    {
        message_body_t() = default;
        // noncopyable
        message_body_t(const message_body_t&) = delete;
        message_body_t& operator= (const message_body_t&) = delete;

        message_body_t(message_body_t&& other) noexcept
        {
            std::swap(size_, other.size_);
            std::swap(buf_, other.buf_);
        }

        message_body_t& operator= (message_body_t&& other) noexcept
        {
            if (this != &other)
            {
                std::swap(size_, other.size_);
                std::swap(buf_, other.buf_);
            }
            return *this;
        }

        using size_t_  = std::uint16_t;
        using array_t_ = std::array<byte_t, DATA_LIMIT>;
        
        /** Результат чтения/записи.
         *  success - bool - флаг успешности операции
         *  read/write - std::size_t - количество обработанных байт
         *  free_space - std::size_t - количество свободных байт в буфере сообщения
         */
        using rw_result_t = std::tuple<bool, std::size_t, std::size_t>;

        size_t_  size_ {0}; // size of payload [2 bytes]
        array_t_ buf_  {0}; // raw bytes       [52 bytes] - DATA_LIMIT

        template <typename data_t>
        inline rw_result_t write(data_t& src, std::size_t count) noexcept
        {
            std::cout << "[msg|body|write] " << count << " bytes into buffer\n";
            const bool check = ((DATA_LIMIT - size_) >= count);
            #ifdef DEBUG
                 // TODO: cout -> log
                assert(check);
            #else
                if (!check) {
                    std::cout << "[msg|body|write] error: bad_size [" << count << " > " << size_ << "]\n";
                    return {false, 0, DATA_LIMIT - size_};
                }
            #endif

            const std::size_t offset = size_;
            engine::util::memcpy(buf_.data() + offset, &src, count);
            size_ += count;
            const std::size_t free_space = DATA_LIMIT - size_;
            return {true, count, free_space};
        }

        template <typename data_t>
        inline rw_result_t read(data_t& dst, std::size_t count) noexcept
        {
            std::cout << "[msg|body|read] " << count << " bytes from buffer\n";
            const bool check = (size_ >= count);
            #ifdef DEBUG
                // TODO: cout -> log
                assert(check);
            #else
                if (!check) {
                    std::cout << "[msg|body|read] error: bad_size [" << count << " > " << size_ << "]\n";
                    return {false, 0, DATA_LIMIT - size_};
                }
            #endif

            const std::size_t offset = size_ - count;
            engine::util::memcpy(&dst, buf_.data() + offset, count);
            std::memset(buf_.data() + offset, 0x0, count);
            size_ -= count;
            const std::size_t free_space = DATA_LIMIT - size_;
            return {true, count, free_space};
        }

        friend std::ostream& operator<< (std::ostream& os, const message_body_t& body)
        {
            std::ios_base::fmtflags f(os.flags());
            os << "data:\n"
                << "\tsize - " << body.size_ << ", free - " << (DATA_LIMIT - body.size_) << "\n"
                << "\tpayload - ";
                if (!body.size_)
                    os << "<empty>";
                else 
                    for (auto& elem: body.buf_) os << std::hex << elem << " ";
                os << "\n";
                os.flags(f);

            return os;
        }
    }; // message_body_t

    /**
     *  Cетевое сообщение (пакет).
     *  Состоит из заголовка, описывающего тип сообщения и
     *  тела сообщения, содержащего полезную нагрузку.
     *  Размер - 64 байт
     */
    template <typename message_id_t>
    struct message_block_t
    {
        message_block_t() = default;
        // noncopyable
        message_block_t(const message_block_t<message_id_t>&) = delete;
        message_block_t<message_id_t>& operator= (const message_block_t<message_id_t>&) = delete;

        message_block_t(message_block_t&& other) noexcept
        {
            std::swap(header, other.header);
            std::swap(body, other.body);
        }

        message_block_t<message_id_t>& operator= (message_block_t<message_id_t>&& other) noexcept
        {
            if (this != &other)
            {
                std::swap(header, other.header);
                std::swap(body, other.body);
            }
            return *this;
        }

        message_header_t<message_id_t>  header {};  // [10 byte]
        message_body_t                  body   {};  // [54 bytes]

        template <typename data_t>
        message_body_t::rw_result_t
        insert(data_t& src) noexcept
        {
            return body.write(src, sizeof(data_t));
        }

        template <typename data_t>
        message_body_t::rw_result_t
        insert(data_t& src, std::size_t n) noexcept
        {
            return body.write(src, n);
        }

        template <typename data_t>
        message_body_t::rw_result_t 
        extract(data_t& dst) noexcept
        {
            return body.read(dst, sizeof(data_t));
        }

        template <typename data_t>
        message_body_t::rw_result_t
        extract(data_t& dst, std::size_t n) noexcept
        {
            return body.read(dst, n);
        }

        [[nodiscard]]
        std::size_t get_payload_size() const
        {
            return body.size_;
        }

        friend std::ostream& operator<< (std::ostream& os, const message_block_t<message_id_t>& msg)
        {
            os << "********** message **********\n"
                << msg.header
                << msg.body
                << "*****************************\n";
            return os;
        }
    }; // message_block_t
} // engine::net::message

namespace engine::net::client
{
    class IClient
    {

    };
}

namespace engine::net::server
{
    /**
     * Интерфейс сервера
     */
    class IServer
    {

    };
}



namespace engine::net::connection
{
    static constexpr std::uint16_t SERVER_PORT = 5000;

    namespace net = boost::asio;

    enum class role_t : std::uint8_t
    {
        CLIENT = 0,
        SERVER
    };

    /**
     * Сокет-пара для каждого соединения.
     */
    class SocketPair
    {
    public:
        explicit SocketPair(net::io_context& context)
                : tcp_socket(context)
                , udp_socket(context)
        {}

        virtual ~SocketPair()
        {
            this->shutdown();
        }

        void shutdown() {
            // close sockets gracefully..
            tcp_socket.shutdown(net::ip::tcp::socket::shutdown_both);
            tcp_socket.close();
            udp_socket.shutdown(net::ip::udp::socket::shutdown_both);
            udp_socket.close();
        }

        net::ip::tcp::socket tcp_socket;
        net::ip::udp::socket udp_socket;
    };

    /**
     * Класс-структура с описанием подключения к удаленному узлу.
     */
    class ConnectionInfo
    {
    public:
        std::uint32_t   id {0};                 // connection id
        std::string     hostName;               // ...¯\_(ツ)_/¯
        std::string     ip {"127.0.0.1"};    //
        std::uint16_t   port {5000};            //
        std::string     description;            //
        role_t          role {role_t::SERVER};  // connection behaviour type
    }; // ConnectionInfo

    /**
     * Класс реализующий соединение между двумя удаленными узлами.
     */
    template <typename message_id_t>
    class Connection final : public std::enable_shared_from_this<Connection<message_id_t>>
    {
    private:
        net::io_context&        context_;
        SocketPair              sockets_;
        net::ip::tcp::acceptor  acceptor_;
        ConnectionInfo          info_;

    public:
        explicit Connection(net::io_context& context, const std::string& host, const std::uint16_t port)
                : context_(context)
                , sockets_(context)
                , acceptor_(context, net::ip::tcp::endpoint(net::ip::tcp::v4(), SERVER_PORT))
        {
            info_.ip = host;
            info_.port = port;
            std::cout << "[" << getRoleStr() << "] created\n";
        }

        ~Connection()
        {
            this->disconnect();
            std::cout << "[" << getRoleStr() << "] closed\n";
        }


    private:
        using connection_handler = std::function<void(boost::system::error_code&)>;
        /**
         * Начать прослушивание порта на подключение клиентов.
         */
        inline void listen(connection_handler handler = nullptr);

        /**
         * Подключиться к удаленному хосту (серверу).
         * @return bool/id - флаг успешности подключения и id соединения, в противном случае false/0
         */
        inline std::pair<bool, std::uint32_t> connect();

        /**
         * Сгенерировать ID соединения
         */
        std::uint32_t generateID();

    public:
        /**
         * Создать "подключение" с удаленным хостом.
         * @return
         */
        std::pair<bool, std::uint32_t> create();

        /**
         * Закрыть соединение с удаленным хостом.
         */
        void disconnect();

        /**
         * Проверить активно ли соединение.
         * @return true/false
         */
        bool isConnected() const;

        /**
         * Вернуть ID соединения.
         */
        std::uint32_t getID() const;

        /**
         * Вернуть тип соединения в виде строки.
         */
        std::string getRoleStr() {
            return info_.role == role_t::CLIENT ? "client" : "server";
        }

    }; // Connection

    template <typename message_id_t>
    std::pair<bool, std::uint32_t> Connection<message_id_t>::create()
    {
        if (role_t::SERVER == info_.role)
        {
            listen();
            return {true, info_.id};
        }
        else
        {
            auto [res, id] = connect();
            return {res, id};
        }
    }

    template <typename message_id_t>
    std::pair<bool, std::uint32_t>
    Connection<message_id_t>::connect()
    {
        net::ip::tcp::resolver resolver(context_);
        auto endpoints = resolver.resolve(info_.ip, std::to_string(info_.port));

        // make connection via tcp
        std::cout << "[client] connecting to server..\n";
        boost::system::error_code ec;
        net::connect(sockets_.tcp_socket, endpoints, ec);
        if (!ec)
        {
            std::cout << "[client] connected to " << sockets_.tcp_socket.remote_endpoint() << "\n";

            // make udp resolver too

            // generate connection (net-user id)
            generateID();

            // start read incoming messages [ASIO TASK]
            // read_tcp();
            // read_udp();

            return  {true, info_.id};
        }
        // error occured
        std::cerr << "[client] connection error - " << ec.message() << "\n";
        // ...
        return {false, 0};
    }

    template <typename message_id_t>
    void Connection<message_id_t>::listen(connection_handler handler) {
        std::cout << "[server] start accepting new connection\n";
        acceptor_.async_accept(
            [this, handler](std::error_code ec, net::ip::tcp::socket socket){
                // приняли новое соединение
                if (!ec)
                {


                    // check client
//                    if (nullptr != handler)
//                        handler();

                    // start udp accepting
                    sockets_.tcp_socket = std::move(socket);
                    info_.ip =

                    generateID();

                    std::cout << "[server] client " << info_.id
                              << " connected on " << info_.ip << ":" << info_.port << "\n";

                    // read_tcp
                    // read_udp
                }
                // произошла ошибка во время установления нового соединения
                else
                {
                    std::cerr << "[server] error occured while establishing new connection - " << ec.message() << "\n";
                }

                // продолджаем слушать новые входящие запросы на подключение [ASIO TASK]
                this->listen();

            } // handler
        ); // async_accept
    }

    template <typename message_id_t>
    void Connection<message_id_t>::disconnect() {
        if (isConnected()) {
            net::post(context_, [this]{
                // try to notify remote endpoint

                // close sockets
                sockets_.shutdown();
            });
        }
    }

    template <typename message_id_t>
    bool Connection<message_id_t>::isConnected() const {
        return sockets_.tcp_socket.is_open() && sockets_.udp_socket.is_open();
    }

    template <typename message_id_t>
    std::uint32_t Connection<message_id_t>::getID() const
    {
        return info_.id;
    }

    template <typename message_id_t>
    std::uint32_t Connection<message_id_t>::generateID() {
        //TODO: hash of uuid; below just for test
        srand((unsigned) time(NULL));
        const auto id = 1 + (rand() % 1024);
        info_.id = id;
        return id;
    }

} // engine::net::connection

namespace engine::net::manager
{
    namespace {
        // pre-defined
//        class Connection;
//        class ConnectionInfo;
        using namespace engine::net::connection;
        using namespace engine::net::message;

        // typedef
        namespace net = boost::asio;
        using dummy_work = std::unique_ptr<net::io_context::work>;
        using connection_table = std::map<std::uint32_t, std::shared_ptr<Connection<message_id>>>;
    }

    /**
     * Менеджер сетевых подключений - обёртка над boost:asio.
     * Входная точка для всех сетевых манипуляций приложения.
     */
    class NetworkManager : boost::noncopyable
    {
    private:
        net::io_context             context_;           // main network event context
        dummy_work                  work_{nullptr};     // dont stop the io!
        std::thread                 context_thread_;    // context thread
        boost::system::error_code   err_;               // manager startup error
        connection_table            connections;        // application endpoints connections

    public:
        NetworkManager(/*config*/)
        {
            work_ = std::make_unique<net::io_context::work>(context_);
            context_thread_ = std::thread([this]{ context_.run(err_); });
        }

        ~NetworkManager()
        {
            work_.reset();
            context_.stop();

            if (context_thread_.joinable())
                context_thread_.join();
        }
    public:
        /**
         * Создать подключение с удалённым узлом.
         * @param host - название хоста или ip адрес
         * @param port - порт
         * @return bool/id - флаг успешности подключения и id соединения, в противном случае false/0
         */
        std::pair<bool, std::uint32_t> createConnection(const std::string& host, const std::uint16_t port);

        /**
         * Получить доступ к соединению по его id.
         * @param id - идентификатор активного соединения
         * @return - ссылка на объект соединения
         */
        template<typename message_id_t>
        std::shared_ptr<Connection<message_id_t>> getConnection(std::uint32_t id);

        net::io_context& getContext() {
            return work_->get_io_context();
        }

        std::pair<int, std::string> getError() {
            return {err_.value(), err_.message()};
        }
    }; // NetworkManager

    std::pair<bool, std::uint32_t>
    NetworkManager::createConnection(const std::string& host, const std::uint16_t port) {
        using ConnectionType = engine::net::connection::Connection<engine::net::message::message_id>;
        // создаём объект подключения
        auto connecion = std::make_shared<ConnectionType>(context_, host, port);
        assert(connecion);

        connecion->create();

        return {false, 0};
    }

} // namespace engine::net::manager

namespace engine::net::detail
{

} // engine::net::detail

// test struct
struct point_t {
    int x = 0;
    int y = 0;

    friend std::ostream& operator<< (std::ostream& os, const point_t& p) {
        os << "{" << p.x << ", " << p.y << "}";
        return os;
    }
};



int main()
{
    std::cout << "std::size_t - " << sizeof(std::size_t) << "\n";
    std::cout << "std::uint32_t - " << sizeof(std::uint32_t) << "\n";

    const auto time_point = std::chrono::high_resolution_clock::now();
    const auto t_c = std::chrono::high_resolution_clock::to_time_t(time_point);
    std::cout << "Now: " << std::put_time(std::localtime(&t_c), "%F %T.") << "\n\n" << std::flush;

    //engine::net::MessageBody mb;
    //engine::game::game_loop();

    /* ******************************************************************************************* */

    /* structures aligns and offsets */
    using namespace engine::net::message;

    message_id id_ {};
    message_header_t<message_id> header_ {};
    message_body_t body_ {};

    message_block_t<message_id> msg {};
    msg.header.type.action = message_id::action_t::PING;

    std::string str {"Times go past whatever is going on...000000000 0x01"};

    SIZEOF(time_point);
    SIZEOF(id_);
    SIZEOF(header_);
    SIZEOF(body_);
    SIZEOF(msg);
    SIZEOF(str);

    message_id id;
    id.action = message_id::action_t::ACTION;
    id.protocol = message_id::proto_t::UDP;
    SIZEOF(id);
    std::cout << "message_id: " << id << "\n\n";

    std::cout << "offsetof type in header: " << offsetof(message_header_t<message_id>, type) << "\n";
    std::cout << "offsetof body in message: " << offsetof(message_block_t<message_id>, body) << "\n";

    /* placement_new messages */

    // buf on stack with placement new
    alignas(message_block_t < message_id >) byte_t message_buffer[1024];
    std::cout << "sizeof message_buffer: " << sizeof(message_buffer) << ", locate at - " << &message_buffer << "\n";

    // message that locates at stack in buf
    message_block_t<message_id>* p_msg = new (message_buffer) message_block_t<message_id>;
    p_msg->header.type.action = message_id::action_t::PING;
    p_msg->header.type.protocol = message_id::proto_t::UDP;
    p_msg->header.uuid = 42;
    p_msg->header.block_num = 1;
    p_msg->header.block_count = 1;
    p_msg->header.access_code = 0xCD;
    p_msg->header.encode = true;
    p_msg->header.compress = false;
    p_msg->body.buf_ = std::array<byte_t, DATA_LIMIT>{0x0, 0x1, 0x2, 0x3, 0x4};

    std::cout << "pMsg:\n" << *p_msg << "\n";
    std::cout << "sizeof placement_new_msg: " << sizeof(*p_msg) << ", locate at - " << p_msg << "\n";

    message_block_t<message_id>* p_msg2 = new (message_buffer) message_block_t<message_id>;
    std::cout << "sizeof placement_new_msg2: " << sizeof(*p_msg2) << ", locate at - " << p_msg2 << "\n";
    std::cout << "test check: " << p_msg2->header.access_code << "\n";
    std::cout << "            " << p_msg2->header.block_num << "\n";


    // buf on heap with operator new() function
    const int msg_count_limit = 10;
    void * buf = operator new(msg_count_limit * sizeof(message_block_t < message_id >));
    std::memset(buf, 0x0, sizeof(buf));
    // message that locates at heap in buf
    auto* p_msg3 = new (buf) message_block_t<message_id>;
    p_msg3->header.type.action = message_id::action_t::ACTION;
    p_msg3->header.type.protocol = message_id::proto_t::TCP;
    SIZEOF(message_buffer);
    SIZEOF(buf);
    std::cout << "sizeof heap" << " msg: " << sizeof(*p_msg3) << ", locate at - " << p_msg3 << "\n";
    std::cout << "msg3:\n" << *p_msg3 << "\n";

    p_msg->~message_block_t<message_id>();
    p_msg2->~message_block_t<message_id>();
    p_msg3->~message_block_t<message_id>();

    operator delete(buf);

    /* placement new MessageQueue */
    {
        using heap_buffer = engine::memory::raw_memory_t<byte_t>;
        using net_message = engine::net::message::message_block_t<engine::net::message::message_id>;

        // allocate 1 kB buffer for net message placing
        const std::size_t size = 1024;    // 1 Kb
        alignas(net_message) heap_buffer buffer(size);
        std::cout << "sizeof buffer - \"" << size << "\",\tplaces at: " << &buffer << "\n";

        // construct net messages
        for (std::size_t i = 0; i < size; i += 64) {
            auto netMessage = new(*buffer) net_message();
            netMessage->header.type.action = message_id::action_t::CHAT_MESSAGE;
            netMessage->header.type.protocol = message_id::proto_t::UDP;
            netMessage->header.access_code = 0x42;
            netMessage->header.uuid = 144;

            std::string s {"number - "};
            s.append(std::to_string(i));
            auto pText = s.c_str();
            netMessage->insert(pText, s.length());
        }

        // destruct net messages
        for (std::size_t i = 0; i < size; i+=64) {
            auto pMsg = reinterpret_cast<net_message*>(buffer+i);
            pMsg->~net_message();
        }

        using namespace engine::net::queue;
        using array = std::array<byte_t, 64>;
        auto queue = std::make_unique<MessageQueue<array>>();

    }


    /* working with message: insert and extract data */
    std::cout << "\n";

    point_t point1 {33, 116};
    point_t point2 {22, 62};
    SIZEOF(point1);

    char s1[] = "12345678!";
    SIZEOF(s1);

    message_block_t<message_id> tmp_msg{};
    SIZEOF(tmp_msg);

    std::cout << "\nTry to write into message:\n";
    {
        const auto&[success, write, free] = tmp_msg.insert(s1);
        std::cout << "[" << success << "/" << write << "/" << free <<"]\n";
    }
    {
        const auto&[success, write, free] = tmp_msg.insert(point1);
        std::cout << "[" << success << "/" << write << "/" << free <<"]\n";
    }
    {
        const auto&[success, write, free] = tmp_msg.insert(point2);
        std::cout << "[" << success << "/" << write << "/" << free <<"]\n";
    }
    // write failure
    {
        char s_fail[29]; // 29 > 28 !!! -> bad_size
        const auto&[success, write, free] = tmp_msg.insert(s_fail);
        std::cout << "[" << success << "/" << write << "/" << free <<"]\n";
    }

    std::cout << "tmp_msg:\n" << tmp_msg << "\n";


    char s2[10];
    point_t point3 {};
    {
        const auto&[success, read, free] = tmp_msg.extract(point3);
        std::cout << "[" << success << "/" << read << "/" << free <<"]\n";
    }
    {
        const auto&[success, read, free] = tmp_msg.extract(point3);
        std::cout << "[" << success << "/" << read << "/" << free <<"]\n";
    }

    {
        const auto&[success, read, free] = tmp_msg.extract(s2, 10);
        std::cout << "[" << success << "/" << read << "/" << free <<"]\n";
    }
    // test read failure
    {
        char s_fail[5];
        const auto&[success, read, free] = tmp_msg.extract(s_fail);
        std::cout << "[" << success << "/" << read << "/" << free <<"]\n";
    }

    std::cout << "tmp_msg:\n" << tmp_msg << "\n";

    std::cout << "\nCompare result:\n";
    std::cout << "s1 - " << s1 << "\n";
    std::cout << "s2 - " << s2 << "\n";
    std::cout << "point1: " << point1 << "\n";
    std::cout << "point2: " << point2 << "\n";
    std::cout << "point3: " << point3 << "\n";



    {
        /* working with std::string via const char* pointer */
        std::cout << "\n\n";

        auto netMessage = std::make_unique<message_block_t < message_id>>();
        const char* pMsgText {nullptr};
        const std::string chatMessage1 {"what a nice net message (:"};

        std::cout << "netMessage after create:\n" << *netMessage.get() << "\n";
        std::cout << "client 1 send msg: \"" << chatMessage1 << "\" [" << chatMessage1.length() << "]\n\n";

        pMsgText = chatMessage1.data();
        netMessage->insert(pMsgText, chatMessage1.length());
        std::cout << *netMessage.get() << "\n";
        
        netMessage->extract(pMsgText, netMessage->get_payload_size());
        std::cout << *netMessage.get() << "\n";
        
        std::string chatMessage2 {pMsgText};        
        std::cout << "size of 'chatMessage2' - " << chatMessage2.size() <<", length - " << chatMessage2.length() << "\n";
        std::cout << "client 2 recieved chat msg: \"" << chatMessage2.c_str() << "\"\n\n";
    }


    /* test connection class */
//    boost::asio::io_context context;
//    auto conn_ptr = std::make_shared<engine::net::connection>(context);
//    conn_ptr->showHostName();
    auto pNetworkMaganer = std::make_unique<engine::net::manager::NetworkManager>();
    {
        const auto& [errCode, errText] = pNetworkMaganer->getError();
        std::cout << "NetworkManager startup status: " << errCode << " - " << errText << "\n";
    }

    using conn_t = engine::net::connection::Connection<message_id>;
    auto conn = std::make_shared<conn_t>(pNetworkMaganer->getContext(), "127.0.0.1", 5000);


    return 0;
}
