#pragma once

#include <boost/noncopyable.hpp>
#include <cstring>
#include <memory>

namespace memory
{
    inline void memcpy(void *dst, void *src, std::size_t n) noexcept {
        using ptr = std::uint8_t* ;
        auto dst_ = reinterpret_cast<ptr>(dst);
        auto src_ = reinterpret_cast<ptr>(src);
        assert(nullptr != dst_);
        assert(nullptr != src_);
        for (std::size_t i = 0; i < n; ++i) {
            dst_[i] = src_[i];
        }
    }

    inline void memmove(void *dst, void *src, std::size_t n) noexcept {
        using ptr = std::uint8_t *;
        auto dst_ = reinterpret_cast<ptr>(dst);
        auto src_ = reinterpret_cast<ptr>(src);
        assert(nullptr != dst_);
        assert(nullptr != src_);
        for (std::size_t i = 0; i < n; ++i) {
            std::swap(dst_[i], src_[i]);
            std::memset(src_, 0x0, 1);
        }
    }

    template<typename T>
    class RawMemory : boost::noncopyable {
    private:
        T*          buffer_   { nullptr };
        std::size_t capacity_ { 0 };

        static T* Allocate(std::size_t n) {
            if (0 == n) return nullptr;
            return static_cast<T*>(operator new(n * sizeof(T)));
        }

        static void Deallocate(T* ptr) {
            return operator delete(ptr);
        }

    public:

        RawMemory() = default;

        explicit RawMemory(std::size_t size = 0) noexcept {
            buffer_ = Allocate(size);
            capacity_ = size;
            std::memset(buffer_, 0x0, size * sizeof(T));
        }

        RawMemory(const RawMemory&) = delete;
        RawMemory& operator= (const RawMemory&) = delete;

        RawMemory(RawMemory&& other)  noexcept {
            if (this != &other) {
                swap(other);
            }
        }

        RawMemory& operator= (RawMemory &&other) noexcept {
            if (this != &other) {
                this->swap(other);
            }
            return *this;
        }

        virtual ~RawMemory() {
            Deallocate(buffer_);
            capacity_ = 0;
        }

        void swap(RawMemory &other) {
            std::swap(buffer_, other.buf_);
            std::swap(capacity_, other.size_);
        }

        T* operator+ (std::size_t i) {
            return buffer_ + i;
        }

        const T* operator+ (std::size_t i) const {
            return buffer_ + i;
        }

        T& operator[] (std::size_t i) {
            return buffer_[i];
        }

        const T& operator[] (std::size_t i) const {
            return buffer_[i];
        }

        T* operator* () {
            return buffer_;
        };

    }; // RawMemory
}
