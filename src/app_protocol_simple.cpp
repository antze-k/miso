// (C) unresolved-external@singu-lair.com released under the MIT license (see LICENSE)

#include "miso/protocol/app_protocol_simple.h"

#include <cstring>
#include <limits>

namespace miso {
//------------------------------------------------------------------------------

bool app_protocol_simple::push_out_message(const std::string& message, char* buf, size_t size, size_t& used)
{
    if (message.size() >= 65536 - sizeof(std::uint16_t))
        return false;

    if (size < sizeof(std::uint16_t) + message.length())
        return false;

    used = sizeof(std::uint16_t) + message.length();

    std::uint16_t message_size = (std::uint16_t)message.length();
    memcpy(buf, &message_size, sizeof(std::uint16_t));
    memcpy(buf + sizeof(std::uint16_t), message.c_str(), message.length());
    return true;
}

//------------------------------------------------------------------------------

bool app_protocol_simple::pop_in_message(const char* buf, size_t size, size_t& used, std::string& message)
{
    if (size < sizeof(std::uint16_t) || size > size_t(std::numeric_limits<std::uint16_t>::max()))
        return false;

    std::uint16_t message_size;
    memcpy(&message_size, buf, sizeof(std::uint16_t));
    if (size < sizeof(std::uint16_t) + message_size)
        return false;

    message.assign(buf + sizeof(std::uint16_t), message_size);
    used = sizeof(std::uint16_t) + message_size;
    return true;
}

//------------------------------------------------------------------------------
}
