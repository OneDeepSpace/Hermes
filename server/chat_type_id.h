
#pragma once

#include <cstdint>
#include <algorithm>

namespace app::message::id
{
    /* --------- Chat app type  ------------
     *
     * Describes sample of chatting messages
     * family type;
     *
     * Structure size - 4 bytes
     * ------------------------------------- */

    struct ChatType
    {
        enum class EChatAction : std::uint16_t
        {
            CHAT_ACT_NONE = 0,
            CHAT_ACT_ROOM_NEW,
            CHAT_ACT_ROOM_DEL,
            CHAT_ACT_ROOM_LIST,
            CHAT_ACT_MESSAGE_PUBLIC,
            CHAT_ACT_MESSAGE_PRIVATE,
            CHAT_ACT_MESSAGE_GROUP,
            //...
        };

        // *** reserved 2byte field ***
        enum class EReserved : std::uint16_t
        {
            NONE = 0
        };

        // --------------------------------------------
        EChatAction     action   {};       // [2 bytes]
        EReserved       reserved {};       // [2 bytes]
        // --------------------------------------------

        ChatType() = default;
        ~ChatType() = default;
        // noncopyable
        ChatType(ChatType const&) = delete;
        ChatType& operator= (ChatType const&) = delete;
        // move semantic
        ChatType(ChatType&&) noexcept;
        ChatType& operator= (ChatType&&) noexcept;

        void swap(ChatType& other) noexcept;

        [[nodiscard]] std::string getActionStr() const;

        friend std::ostream& operator<< (std::ostream& os, ChatType const& st);

    }; // ChatType

    ChatType::ChatType(ChatType&& other) noexcept
    {
        this != &other ? this->swap(other) : void(0);
    }

    ChatType& ChatType::operator= (ChatType&& other) noexcept
    {
        this != &other ? this->swap(other) : void(0);
        return *this;
    }

    void ChatType::swap(ChatType& other)  noexcept
    {
        std::swap(action, other.action);
        std::swap(reserved, other.reserved);
    }

    std::ostream& operator<< (std::ostream &os, ChatType const& st) {
        os  << "type:\n"
            << "  action - " << st.getActionStr() << "\n";
        return os;
    }

    std::string ChatType::getActionStr() const
    {
        switch(action) {
            default:
            case EChatAction::CHAT_ACT_NONE:            return "none";
            case EChatAction::CHAT_ACT_ROOM_NEW:        return "room_new";
            case EChatAction::CHAT_ACT_ROOM_DEL:        return "room_del";
            case EChatAction::CHAT_ACT_ROOM_LIST:       return "room_list";
            case EChatAction::CHAT_ACT_MESSAGE_PUBLIC:  return "msg_pub";
            case EChatAction::CHAT_ACT_MESSAGE_PRIVATE: return "msg_prv";
            case EChatAction::CHAT_ACT_MESSAGE_GROUP:   return "msg_grp";
        }
    }
}