
#pragma once

#include <hermes/common/utils.h>

#include <string>
#include <string_view>

#include <boost/uuid/uuid.hpp>
#include <boost/lexical_cast.hpp>

namespace network::message::object
{

    /* --------------- Disconnect message object ----------------
     *
     * Server or client send this object wrapped into net message
     * when one of those should disconnect or be disconnected.
     *
     * Structure size - * bytes
     * ---------------------------------------------------------- */

    class MDisconnect
    {
    public:
        enum class EDisconnectReason : std::uint32_t
        {
            DISCONNECT_DEFAULT = 0,
            DISCONNECT_EXCLUDED,
            DISCONNECT_BANNED,
            DISCONNECT_TIMEOUT,
            DISCONNECT_SERVERFAULT
        };

    private:
        boost::uuids::uuid  uuid_;      // client id
        EDisconnectReason   reason_;    // what happened

    public:
        explicit MDisconnect(boost::uuids::uuid uuid, EDisconnectReason reason)
            : uuid_(uuid)
            , reason_(reason)
        {}

        MDisconnect() = delete;
        ~MDisconnect() = default;

        [[nodiscard]] boost::uuids::uuid getUUID() const {
            return uuid_;
        }

        std::string getUUIDStr() const {
            return boost::lexical_cast<std::string>(uuid_);
        }

        [[nodiscard]] EDisconnectReason getReason() const {
            return reason_;
        }

        std::string getReasonStr() const {
            switch (reason_) {
                default:
                case EDisconnectReason::DISCONNECT_DEFAULT:     return "Default";
                case EDisconnectReason::DISCONNECT_EXCLUDED:    return "Excluded";
                case EDisconnectReason::DISCONNECT_BANNED:      return "Banned";
                case EDisconnectReason::DISCONNECT_TIMEOUT:     return "Timeout";
                case EDisconnectReason::DISCONNECT_SERVERFAULT: return "Server fault";
            }
        }

        friend std::ostream& operator<< (std::ostream& os, MDisconnect const& d) {
            os  << "MDisconnect:\n"
                << "  uuid   - " << d.getUUIDStr() << "\n"
                << "  reason - " << d.getReasonStr() << "\n";
            return os;
        }
    };  // MDisconnect

}   // network::message::object
