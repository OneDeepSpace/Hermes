
#pragma once

#include <cstdint>

namespace network::message::v2::object
{

    /* ------------------- Accept message object ------------------
     *
     * If the client passed the test(protection value), this object,
     * wrapped into net message, is sent to the client to confirm
     * the connection with server.
     *
     * Structure size - * bytes
     * ------------------------------------------------------------ */

    class MAcceptConnect
    {
    private:
        std::uint8_t    clientAccessCode_;  // unique access code for each client messages
        std::uint16_t   port_;              // new associated private port for client

    public:
        explicit MAcceptConnect(std::uint8_t code, std::uint16_t port)
            : clientAccessCode_(code)
            , port_(port)
        {}

        ~MAcceptConnect() = default;

        std::uint8_t getAccessCode() const {
            return clientAccessCode_;
        }

        std::uint16_t getPrivatePort() const {
            return port_;
        }

        friend std::ostream& operator<< (std::ostream& os, MAcceptConnect const& ac) {
            os  << "MAcceptConnect:\n"
                << "  access code: " << ac.clientAccessCode_ << "\n"
                << "  client port: " << ac.port_ << "\n";
            return os;
        }

    };  // MAcceptConnect

} // network::message::object
