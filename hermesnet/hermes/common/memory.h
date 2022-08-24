#pragma once

#include <boost/noncopyable.hpp>
#include <cstring>
#include <memory>

namespace memory
{
    inline void memcpy(void *dst, void *src, std::size_t n) noexcept {
        using ptr = std::uint8_t *;
        auto dst_ = reinterpret_cast<ptr>(dst);
        auto src_ = reinterpret_cast<ptr>(src);
        for (std::size_t i = 0; i < n; ++i) {
            dst_[i] = src_[i];
        }
    }

    inline void memmove(void *dst, void *src, std::size_t n) noexcept {
        using ptr = std::uint8_t *;
        auto dst_ = reinterpret_cast<ptr>(dst);
        auto src_ = reinterpret_cast<ptr>(src);
        for (std::size_t i = 0; i < n; ++i) {
            std::swap(dst_[i], src_[i]);
            std::memset(src_, 0x0, 1);
        }
    }

    template<typename Type>
    class RawMemory : boost::noncopyable {
    private:
        std::size_t cap{0};
        Type *buffer{nullptr};

        static Type *Allocate(std::size_t n) {
            return static_cast<Type *>(operator new(n));
        }

        static void Deallocate(Type *ptr) {
            operator delete(ptr);
        }

    public:
        explicit RawMemory(std::size_t size) noexcept {
            buffer = Allocate(size);
            cap = size;
            std::memset(buffer, 0x0, size * sizeof(Type));
        }

        ~RawMemory() {
            Deallocate(buffer);
            cap = 0;
        }

        void swap(RawMemory &other) {
            std::swap(buffer, other.buf_);
            std::swap(cap, other.size_);
        }

        RawMemory &operator=(RawMemory &&other) noexcept {
            this->swap(other);
        }

        Type *operator+(std::size_t i) {
            return buffer + i;
        }

        const Type *operator+(std::size_t i) const {
            return buffer + i;
        }

        Type &operator[](std::size_t i) {
            return buffer[i];
        }

        const Type &operator[](std::size_t i) const {
            return buffer[i];
        }

        Type *operator*() {
            return buffer;
        };
    }; // RawMemory
}
