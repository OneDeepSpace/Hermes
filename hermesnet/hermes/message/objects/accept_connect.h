
#pragma once

#include <cstdint>
#include <iomanip>
#include <iostream>

namespace network::message::object
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

            auto asHex = [&os](std::uint8_t b) -> void {
                os << "0x" << std::hex << std::setw(2) << std::setfill('0') << static_cast<unsigned>(b) << std::dec;
            };

            os  << "MAcceptConnect:\n"
                << "  access code: " ; asHex(ac.clientAccessCode_); os << "\n"
                << "  client port: " << ac.port_ << "\n";
            return os;
        }

    };  // MAcceptConnect

} // network::message::object
