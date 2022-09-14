
#pragma once

#include "header.h"
#include "body.h"

#include <utility>

namespace network::message
{

    /* ------------- Datagram  ------------
     *
     * Contains header and body for network
     * communication between nodes (client/server);
     * Uses continuous buffer with constant size
     * for DOD (SoA) features.
     *
     * Structure size - 64 bytes
     * ------------------------------------ */

    template <typename IdType>
    class Datagram
    {
    private:
        // ---------------------------------------------------
        Header<IdType>  header_ {};              // [10 bytes]
        Body            body_   {};              // [54 bytes]
        // ---------------------------------------------------

        using BufferType = Body::BufferType;
        using SizeType   = Body::SizeType;

    public:
        Datagram() = default;
        explicit Datagram(Header<IdType>&& h, Body&& b) noexcept;
        ~Datagram() = default;
        // noncopyable
        Datagram(Datagram const&) = delete;
        Datagram& operator= (Datagram const&) = delete;
        // move semantic
        Datagram(Datagram&& ) noexcept;
        Datagram& operator= (Datagram&& ) noexcept;

        template <class U>
        friend std::ostream& operator<< (std::ostream& os, Datagram& d);

    public:
        Header<IdType>& HeaderRef();
        Body& BodyRef();
        BufferType& Data();
        SizeType getDataSize() const;

        void swap(Datagram&) noexcept;

    };  // Datagram

    // ********************************* IMPLEMENTATION **********************************

    template <typename IdType>
    Datagram<IdType>::Datagram(Header<IdType>&& h, Body&& b) noexcept
        : header_(std::forward<decltype(h)>(h))
        , body_(std::forward<decltype(b)>(b))
    {}

    template <typename IdType>
    Datagram<IdType>::Datagram(Datagram&& other) noexcept
    {
        this != &other ? this->swap(other) : void(0);
    }

    template <typename IdType>
    Datagram<IdType>& Datagram<IdType>::operator= (Datagram&& other) noexcept
    {
        this != &other ? this->swap(other) : void(0);
        return *this;
    }

    template <typename IdType>
    void Datagram<IdType>::swap(Datagram &d) noexcept
    {
        std::swap(header_, d.header_);
        std::swap(body_, d.body_);
    }

    template <class IdType>
    std::ostream& operator<< (std::ostream& os, Datagram<IdType>& d) {
        os  << "[Datagram]\n"
            << d.HeaderRef()
            << d.BodyRef()
            << "\n";
        return os;
    }

    template <typename IdType>
    Header<IdType>& Datagram<IdType>::HeaderRef()
    {
        return header_;
    }

    template <typename IdType>
    Body& Datagram<IdType>::BodyRef()
    {
        return body_;
    }

    template <typename IdType>
    typename Datagram<IdType>::BufferType& Datagram<IdType>::Data()
    {
        return body_.buf;
    }

    template <typename IdType>
    typename Datagram<IdType>::SizeType Datagram<IdType>::getDataSize() const
    {
        return body_.size;
    }

}   // network::message

