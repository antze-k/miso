// (C) unresolved-external@singu-lair.com released under the MIT license (see LICENSE)

#include "miso/server/server_udp.h"
#include "miso/socket/generic_socket.h"

namespace miso {
//------------------------------------------------------------------------------

bool server_udp::open(std::uint16_t port)
{
    generic_socket s;
    if (!s.configure(ipv4, udp, nonblocking, true))
        return false;

    return open(std::move(s));
}

//------------------------------------------------------------------------------

bool server_udp::open(generic_socket&& s)
{
    return false;
}

//------------------------------------------------------------------------------
}
