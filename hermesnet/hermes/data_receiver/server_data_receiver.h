#pragma once

#include <cstdint>

#include "interface/ireceiver.h"
#include <hermes/common/types.h>
#include <hermes/common/structures.h>

#include <boost/noncopyable.hpp>
#include <boost/asio/ip/udp.hpp>

namespace network::service
{
#ifndef ASIO_TYPEDEF
#define ASIO_TYPEDEF
    namespace net = boost::asio;
#endif

    class ServerDataReceiver final : public IReceiver, boost::noncopyable
    {
    private:
        using AssociatedChunk = std::pair<net::ip::udp::endpoint, std::vector<std::uint8_t>>;

        class Entry&    refEntry_;
        class Clients&  refClients_;

    public:
        explicit ServerDataReceiver(net::io_service& service, Entry& e, Clients& c);
        virtual ~ServerDataReceiver() = default;

        // Обработать входящие сообщения
        void process() final;

    private:
        // Получить количество доступных байт для чтения без блокировки
        inline std::size_t isDataReady(const boost::asio::ip::udp::socket& socket) final;

        // Проверить доступ сообщения
        //inline bool validateData(std::vector<std::uint8_t>& data, std::uint8_t code);

        // Прочитать ассоциированные с удаленной точкой данные с входного сокета сервера
        std::vector<AssociatedChunk> readFromEntrySocket(net::ip::udp::socket& socket, std::uint8_t code);
        // Прочитать данные от всех клиентов
        std::vector<std::uint8_t>
        readFromClientSockets(std::vector<net::ip::udp::socket>& sockets, std::vector<std::size_t>& sizes, std::vector<std::uint8_t>& codes);
    };
}


