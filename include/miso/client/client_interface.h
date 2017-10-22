// (C) unresolved-external@singu-lair.com released under the MIT license (see LICENSE)

#pragma once

#include <cstdint>
#include <string>

namespace miso {
//------------------------------------------------------------------------------

class client_interface
{
public:
    virtual bool send_raw(const void* data, std::uint16_t size) = 0;
    virtual std::uint16_t recv_raw(void* data, std::uint16_t size) = 0;
};

//------------------------------------------------------------------------------
}
