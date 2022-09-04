
#include <vector>
#include <atomic>
#include <memory>
#include <utility>
#include <algorithm>
#include <hermes/common/memory.h>


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

    // Helper functions

    static void Construct(void * ptr) {
        new(ptr) MessageType;
    }

    static void Construct(void * ptr, const MessageType& elem) {
        new(ptr) MessageType(elem);
    }

    static void Construct(void * ptr, MessageType&& elem) {
        new(ptr) MessageType(std::forward<decltype(elem)>(elem));
    }

    static void Destroy(MessageType* ptr) {
        ptr->~MessageType();
    }

private:
    //-------------------------------------------------------------
    RawMemory<MessageType>  data_           {};
    std::size_t             size_           { 0 };
    std::atomic_bool        bReadyToRead    { false };
    std::atomic_bool        bReadyToWrite   { false };
    //-------------------------------------------------------------

    friend RawMemory<MessageType>;

public:

    ExchangeBuffer() = default;

    explicit ExchangeBuffer(std::size_t n)
            : data_(n)
    {
        std::uninitialized_value_construct_n(data_.buffer_, n);
        this->size_ = n;
    }

    ExchangeBuffer(const ExchangeBuffer& other)
            : data_(other.size_)
    {
        std::uninitialized_copy_n(other.data_.buffer_, other.size_, data_.buffer_);
        this->size_ = other.size_;
    }

    ExchangeBuffer(ExchangeBuffer&& other) noexcept
    {
        swap(other);
    }

    ExchangeBuffer& operator= (const ExchangeBuffer& other)
    {
        if (other.size_ > data_.capacity_)
        {
            ExchangeBuffer tmp(other);
            swap(other);
        }
        else
        {
            for (std::size_t i = 0; i < size_ and i < other.size_; ++i)
                data_[i] = other[i];

            if (size_ < other.size_)
            {
                std::uninitialized_copy_n(other.data_.buffer_ + size_, other.size_ - size_, data_.buffer_ + size_);
            }
            else if (size_ > other.size_)
            {
                std::destroy_n(data_.buffer_ + other.size_, size_ - other.size_);
            }

            size_ = other.size_;
        }

        return *this;
    }

    ExchangeBuffer& operator= (ExchangeBuffer&& other)
    {
        swap(other);
        return *this;
    }

    void reserve(std::size_t n)
    {
        if (n <= data_.capacity_) return;

        RawMemory<MessageType> newData(n);

        std::uninitialized_move_n(data_.buffer_, size_, newData.buffer_);
        std::destroy_n(data_.buffer_, size_);

        data_.swap(newData);
    }

    void swap(ExchangeBuffer& other)
    {
        data_.swap(other.data_);
        std::swap(size_, other.size_);
    }

    virtual ~ExchangeBuffer()
    {
        std::destroy_n(data_.buffer_, size_);
    }

    [[nodiscard]] std::size_t size() const
    {
        return size_;
    }

    [[nodiscard]] std::size_t capacity() const
    {
        return data_.cap_;
    }

    void insert(const MessageType& elem)
    {
        new(data_ + size_) MessageType(elem);
        ++size_;
    }

    // добавить элемент в конец массива
    void insert(MessageType&& elem)
    {
        new(data_ + size_) MessageType(std::forward<decltype(elem)>(elem));
        ++size_;
    }

    // получить элемент с конца массива
    MessageType extract()
    {
        auto lastElem = std::move(data_[size_ - 1]);
        std::destroy_at(data_[size_ - 1]);
        --size_;
        return std::move(lastElem);
    }

    std::vector<MessageType> extractAll()
    {
        std::vector<MessageType> result;
        result.reserve(size_);

        auto f = [this, &result](const MessageType& elem) {
            result.emplace_back(std::move(this->extract(elem)));
        };

        std::for_each(data_.buffer_, data_.buffer_ + size_, f);
        size_ = 0;

        return result;
    }

};  // buffer