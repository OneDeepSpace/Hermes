
#pragma once

#include <hermes/common/utils.h>

#include <string>
#include <cassert>
#include <string_view>

#include <boost/uuid/uuid.hpp>
#include <boost/lexical_cast.hpp>

namespace network::message::v2::object
{

    /* ----------------- Connect message object ------------------
     *
     * Clint send this object wrapped into net message to
     * try to connect server with protection value for got trusted.
     *
     * Structure size - * bytes
     * ----------------------------------------------------------- */

    class MConnect
    {
    private:
        std::uint32_t       prot_   ;   // protection value
        std::uint32_t       version_;   // app version
        boost::uuids::uuid  uuid_;      // id

    public:
        explicit MConnect(std::uint32_t protectionValue) noexcept
            : prot_(protectionValue)
            , version_(1)   // temp
        {
            uuid_ = utility::misc::generateUUID();
        }

        MConnect() = delete;
        ~MConnect() = default;

        [[nodiscard]] std::uint32_t getProtectionValue() const {
            return prot_;
        }

        void setUUID(boost::uuids::uuid nUUID) {
            uuid_ = nUUID;
        }

        [[nodiscard]] boost::uuids::uuid getUUID() const {
            return uuid_;
        }

        [[nodiscard]] std::string getUUIDStr() const {
            return boost::lexical_cast<std::string>(uuid_);
        }

        friend std::ostream& operator<< (std::ostream& os, MConnect const& c) {
            os  << "MConnect:\n"
                << "  prot - " << c.prot_ << "\n"
                << "  ver. - " << c.version_ << "\n"
                << "  uuid - " << c.getUUIDStr() << "\n";
            return os;
        }

    }; // MConnect

}   // network::message::object
