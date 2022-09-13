#pragma once

#include <chrono>
#include <vector>
#include <string_view>

#include <hermes/common/types.h>
#include <hermes/message/helper.h>
#include <hermes/message/service_type_id.h>
#include <hermes/message/datagram.h>

#include <hermes/message/objects/ping.h>
#include <hermes/message/objects/connect.h>
#include <hermes/message/objects/disconnect.h>
#include <hermes/message/objects/accept_connect.h>
#include <hermes/message/objects/decline_connect.h>


namespace network::message
{

    /*

    template <typename Type, typename Data>
    std::vector<Datagram<Type>> generate(typename Type::header::type id, Data data)
    {
        std::uint32_t count { 0 };
        {
            const std::uint32_t cached { data.size() };
            auto preliminary { cached / CAPACITY };
            const bool rest { (cached % CAPACITY) == 0 };
            count = rest ? preliminary : ++preliminary;
        }

        std::vector<Datagram<Type>> messages;
        messages.reserve(count);

        std::uint32_t stored { 0 };
        std::uint32_t to_write { count > 1 ? CAPACITY : data.size() };

        for (std::uint32_t i = 1; i < count + 1; ++i) {
            Datagram<Type> msg;

            msg.header.type = id;
            msg.header.uuid = 0x1;
            msg.header.access_code = SERVER_ACCESS_CODE;
            msg.header.block_num = i;
            msg.header.block_count = count;
            msg.header.encode = false;
            msg.header.compress = false;

            auto&[ok, writed, free] = msg.write_n(data, to_write);
            to_write -= i * CAPACITY - stored;

            messages.emplace_back(std::move(msg));
        }

        return messages;
    }
    */
}
