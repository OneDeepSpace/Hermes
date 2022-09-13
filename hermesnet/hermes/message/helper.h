
#pragma once

#include <hermes/common/types.h>
#include <hermes/message/datagram.h>

// TODO:

namespace network::message::helper
{
    using namespace network::types;


    template <typename IdType>
    void prepareDatagram(Datagram<IdType>& datagram,
                        std::uint8_t code = SERVER_ACCESS_CODE,
                        std::uint8_t last = END_MESSAGE_BYTE) noexcept
    {
        datagram.HeaderRef().access_code = SERVER_ACCESS_CODE;
        datagram.Data()[END_MESSAGE_BYTE_POS] = END_MESSAGE_BYTE;
    }

    template <typename IdType>
    bool validateDataram(Datagram<IdType>& datagram,
                         std::uint8_t code = SERVER_ACCESS_CODE,
                         std::uint8_t last = END_MESSAGE_BYTE) noexcept
    {
        const bool sc { 64 == sizeof(datagram) };
        const bool ac { code == datagram.HeaderRef().access_code };
        const bool ec { last == datagram.Data()[END_MESSAGE_BYTE_POS] };
        return sc and ac and ec;
    }

}   // network::message::helper
