// (C) unresolved-external@singu-lair.com released under the MIT license (see LICENSE)

#pragma once

#include <cstdint>
#include <string>

namespace miso {
//------------------------------------------------------------------------------

class server_interface
{
public:
    using client_id = unsigned int;
private:
    static const client_id _invalid_client = (client_id)-1;
public:
    static inline client_id invalid_client() throw() { return _invalid_client; }

    virtual bool send_raw(client_id id, const void* data, std::uint16_t size) = 0;
    virtual std::uint16_t recv_raw(client_id id, void* data, std::uint16_t size) = 0;
};

//------------------------------------------------------------------------------
}
