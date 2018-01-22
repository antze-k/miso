// (C) unresolved-external@singu-lair.com released under the MIT license (see LICENSE)

#include "miso/protocol/app_protocol_simple.h"

#include <cstring>
#include <limits>

namespace miso {
//------------------------------------------------------------------------------

bool app_protocol_simple::push_out_message(const std::vector<std::uint8_t>& message, char* buf, size_t size, size_t& used)
{
    if (message.size() >= 65536 - sizeof(std::uint16_t))
        return false;

    if (size < sizeof(std::uint16_t) + message.size())
        return false;

    used = sizeof(std::uint16_t) + message.size();

    std::uint16_t message_size = (std::uint16_t)message.size();
    memcpy(buf, &message_size, sizeof(std::uint16_t));
    if (!message.empty())
        memcpy(buf + sizeof(std::uint16_t), &message.front(), message.size());
    return true;
}

//------------------------------------------------------------------------------

bool app_protocol_simple::pop_in_message(const char* buf, size_t size, size_t& used, std::vector<std::uint8_t>& message)
{
    if (size < sizeof(std::uint16_t) || size > size_t(std::numeric_limits<std::uint16_t>::max()))
        return false;

    std::uint16_t message_size;
    memcpy(&message_size, buf, sizeof(std::uint16_t));
    if (size < sizeof(std::uint16_t) + message_size)
        return false;

    message.resize(message_size);
    if (!message.empty())
        memcpy(&message.front(), buf + sizeof(std::uint16_t), message_size);
    used = sizeof(std::uint16_t) + message_size;
    return true;
}

//------------------------------------------------------------------------------
}
