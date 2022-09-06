
#pragma once

#include <chrono>

#include <boost/noncopyable.hpp>
#include <boost/circular_buffer.hpp>

namespace network::buffer
{
    /**
     * Привязка времени прибытия сообщения и самого сообщения.
     */
    template <typename MessageType>
    struct TimedMessage
    {
        TimedMessage() = default;
        explicit TimedMessage(MessageType&& m) noexcept;

        MessageType message;
        std::chrono::high_resolution_clock::time_point arrivedTime;
    };

    /**
     * Кольцевой буфер для накопления входящих и исходящих
     * сообщений для последующей перелачи в буфер обмена или
     * сокет.
     * @tparam ElementType
     */
    template <typename ElementType>
    class MessageBuffer : boost::noncopyable
    {
    public:
        boost::circular_buffer<ElementType> circularBuffer_;

    public:
        // capacity -> new_cap = msgCountCapacity * sizeof(ElemType) bytes
        explicit MessageBuffer(std::size_t msgCountCapacity) noexcept;

        MessageBuffer() = delete;

        MessageBuffer(MessageBuffer&& other) noexcept;
        MessageBuffer& operator= (MessageBuffer&& other) noexcept;

        [[nodiscard]] bool full() const;

        void storeElem(ElementType&& elem);
        void extractAll(std::vector<ElementType>& result);

    };  // MessageBuffer

}   // network::buffer


// ██████  ███████ ███████ ██ ███    ██ ██ ████████ ██  ██████  ███    ██
// ██   ██ ██      ██      ██ ████   ██ ██    ██    ██ ██    ██ ████   ██
// ██   ██ █████   █████   ██ ██ ██  ██ ██    ██    ██ ██    ██ ██ ██  ██
// ██   ██ ██      ██      ██ ██  ██ ██ ██    ██    ██ ██    ██ ██  ██ ██
// ██████  ███████ ██      ██ ██   ████ ██    ██    ██  ██████  ██   ████

#include <hermes/log/log.h>

using namespace utility::logger;

namespace
{
#undef  LOG
#define LOG(text) Logger::getInstance().log(EModule::CIRCBUF, (text));
}

using namespace network::buffer;

template <typename ElementType>
TimedMessage<ElementType>::TimedMessage(ElementType&& m) noexcept
{
    message = std::forward<ElementType>(m);
    arrivedTime = std::chrono::high_resolution_clock::now();
}

template <typename ElementType>
MessageBuffer<ElementType>::MessageBuffer(std::size_t count) noexcept
{
    LOG_REGISTER_MODULE(EModule::CIRCBUF)
    circularBuffer_.set_capacity(count * sizeof(ElementType));
}

template<typename ElementType>
MessageBuffer<ElementType>::MessageBuffer(MessageBuffer &&other) noexcept
{
    if (this != &other)
    {
        circularBuffer_.swap(other.circularBuffer_);
    }
}

template<typename ElementType>
MessageBuffer<ElementType>& MessageBuffer<ElementType>::operator=(MessageBuffer &&other) noexcept
{
    if (this != &other)
    {
        circularBuffer_.swap(other.circularBuffer_);
    }
    return *this;
}

template<typename MessageType>
bool MessageBuffer<MessageType>::full() const
{
    return circularBuffer_.full();
}

template <typename ElementType>
void MessageBuffer<ElementType>::storeElem(ElementType&& elem)
{
#ifdef Debug
    LOG("storing message")
#endif
    circularBuffer_.push_back(std::forward<ElementType>(elem));
}

template <typename ElementType>
void MessageBuffer<ElementType>::extractAll(std::vector<ElementType>& result)
{
#ifdef Debug
    LOG("extract all message from circular buffer")
#endif
    if (not circularBuffer_.empty())
    {
        std::move(circularBuffer_.begin(), circularBuffer_.end(), result.begin());
    }
}

