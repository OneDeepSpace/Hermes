
#pragma once

#include <cstdint>
#include <algorithm>

namespace network::message::id
{

    /* ----------- Service type  ------------
     *
     * Describes service message type family;
     *
     * Structure size - 4 bytes
     * -------------------------------------- */

    struct ServiceType
    {
        enum class EServiceAction : std::uint16_t
        {
            SERVICE_ACT_NONE = 0,
            SERVICE_ACT_PING,
            SERVICE_ACT_CONNECT,
            SERVICE_ACT_ACCEPT,
            SERVICE_ACT_DECLINE,
            SERVICE_ACT_DISCONNECT,
        };

        // *** reserved 2byte field ***
        enum class EReserved : std::uint16_t
        {
            NONE = 0
        };

        // --------------------------------------------
        EServiceAction  action   {};       // [2 bytes]
        EReserved       reserved {};       // [2 bytes]
        // --------------------------------------------

        ServiceType() = default;
        ~ServiceType() = default;
        // noncopyable
        ServiceType(ServiceType const&) = delete;
        ServiceType& operator= (ServiceType const&) = delete;
        // move semantic
        ServiceType(ServiceType&&) noexcept;
        ServiceType& operator= (ServiceType&&) noexcept;

        void swap(ServiceType& other) noexcept;

        [[nodiscard]] std::string getActionStr() const;

        friend std::ostream& operator<< (std::ostream& os, ServiceType const& st);

    }; // ServiceType

    ServiceType::ServiceType(ServiceType&& other) noexcept
    {
        this != &other ? this->swap(other) : void(0);
    }

    ServiceType& ServiceType::operator= (ServiceType&& other) noexcept
    {
        this != &other ? this->swap(other) : void(0);
        return *this;
    }

    void ServiceType::swap(ServiceType& other)  noexcept
    {
        std::swap(action, other.action);
        std::swap(reserved, other.reserved);
    }

    std::ostream& operator<< (std::ostream &os, ServiceType const& st) {
        os << "action - " << st.getActionStr() << "\n";
        return os;
    }

    std::string ServiceType::getActionStr() const
    {
        switch(action) {
            default:
            case EServiceAction::SERVICE_ACT_NONE:          return "none";
            case EServiceAction::SERVICE_ACT_PING:          return "ping";
            case EServiceAction::SERVICE_ACT_CONNECT:       return "connect";
            case EServiceAction::SERVICE_ACT_ACCEPT:        return "accept";
            case EServiceAction::SERVICE_ACT_DECLINE:       return "decline";
            case EServiceAction::SERVICE_ACT_DISCONNECT:    return "disconnect";
        }
    }

}   // network::message::id
