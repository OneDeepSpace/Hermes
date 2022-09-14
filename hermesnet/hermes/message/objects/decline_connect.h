#pragma once

#include <cstdint>

namespace network::message::object
{

    /* ----------------- Decline message object ------------------
     *
     * If the client don't passed the test(protection value), this
     * object, wrapped into net message, is sent to the client to
     * decline the connection with server.
     *
     * Structure size - * bytes
     * ----------------------------------------------------------- */

    class MDeclineConnect
    {
    public:
        enum class EDeclineReason : std::uint32_t
        {
            DECLINE_WRONG_PROTECTION_VALUE = 0,      // protection value incorrect
            DECLINE_WRONG_PASSWORD,                  // given password incorrect
            DECLINE_OUTDATED_CLIENT_VERSION,         // client's app version is out of date
            DECLINE_NO_PLACE,                        // no free space on server
            DECLINE_BANNED,                          // client was banned on server
        };

    private:
        EDeclineReason  reason_;

    public:
        explicit MDeclineConnect(EDeclineReason reason)
            : reason_(reason)
        {}

        ~MDeclineConnect() = default;

        std::string getReasonStr() const {
            switch (reason_) {
                default:
                case EDeclineReason::DECLINE_WRONG_PROTECTION_VALUE:    return "Incorrect protection value";
                case EDeclineReason::DECLINE_WRONG_PASSWORD:            return "Incorrect password";
                case EDeclineReason::DECLINE_OUTDATED_CLIENT_VERSION:   return "Client version is out of date";
                case EDeclineReason::DECLINE_NO_PLACE:                  return "Server if full";
                case EDeclineReason::DECLINE_BANNED:                    return "You banned";
            }
        }

        friend std::ostream& operator<< (std::ostream& os, MDeclineConnect const& dc) {
            os  << "MDeclineConnect:\n"
                << "  reason - " << dc.getReasonStr() << "\n";
            return os;
        }

    };  // MDeclineConnect

} // network::message::object
