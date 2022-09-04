#pragma once

#include <thread>
#include <vector>
#include <atomic>
#include <memory>
#include <utility>
#include <algorithm>

namespace network::buffer
{

    struct ThreadAccessor
    {
        std::thread::id     id              { 0 };
        std::atomic_bool    bReadyToWrite_  { false };
        std::atomic_bool    bReadyToRead_   { false };
    };


    /**
     * Буфер для обмена сетевыми сообщениями в неблокирующем
     * режиме с помощью флагов доступа к данным между сетевым
     * потоком и потоком системы событий, принадлежащий основному
     * потоку приложения.
     */
    template <typename MessageType>
    class ExchangeBuffer
    {
    private:
        std::vector<MessageType>    data_;
        ThreadAccessor              reader_;
        ThreadAccessor              writer_;

    public:


        void writeData();
        void getData();

        void setReaderThread(std::thread& thread)
        {
            reader_.id = thread.get_id();
        }

        void setWriterThread(std::thread& thread)
        {
            writer_.id = thread.get_id();
        }

    };  // buffer

}
