
#pragma once

#include <vector>
#include <hermes/common/types.h>
#include <boost/noncopyable.hpp>

namespace network
{

    class Entry;
    class Clients;

    /*
     * Модуль отправки данных
     */
    class DataSender : boost::noncopyable
    {
    public:
        DataSender();
        virtual ~DataSender() = default;

    public:
        // Обработать исходящие сообщения
        void processOutcomingMessages(Entry& , Clients& , std::vector<std::uint8_t>& );

    };
}


