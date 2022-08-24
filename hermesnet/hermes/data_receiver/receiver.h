
#pragma once

#include <boost/noncopyable.hpp>
#include <boost/asio/ip/udp.hpp>

namespace network
{

#ifndef ASIO_TYPEDEF
#define ASIO_TYPEDEF
    namespace net = boost::asio;
#endif

    class Entry;
    class Clients;

    /*
     * Модуль приёма данных
     */
    class DataReceiver
    {
    public:
        using Socket = net::ip::udp::socket;
        using AssociatedChunk = std::pair<net::ip::udp::endpoint, std::vector<std::uint8_t>>;

        DataReceiver();

    public:
        // Обработать входящие сообщения
        void processIncomingMessages(Entry& , Clients& , std::vector<std::uint8_t>& );

    private:
        // Проверить доступ сообщения
        inline bool validateData(std::vector<std::uint8_t>& data, std::uint8_t code);
        // Получить количество доступных байт для чтения без блокировки с сокета
        inline std::size_t checkAvailableData(const Socket& sock);

        // Прочитать ассоциированные с удаленной точкой данные с входного сокета сервера
        std::vector<AssociatedChunk> readFromEntrySocket(Socket& socket, std::uint8_t code);
        // Прочитать данные от всех клиентов
        std::vector<std::uint8_t>
        readFromClientSockets(std::vector<Socket>& sockets, std::vector<std::size_t>& sizes, std::vector<std::uint8_t>& codes);

    };  // DataReceiver
}