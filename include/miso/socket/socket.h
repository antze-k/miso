// (C) unresolved-external@singu-lair.com released under the MIT license (see LICENSE)

#pragma once

#include "miso/socket/generic_socket.h"

namespace miso {
//------------------------------------------------------------------------------

template<transport_protocol_t transport_protocol>
class socket : public generic_socket
{
public:
    inline socket() throw() { configure(ipv4, transport_protocol, nonblocking); }
};

//------------------------------------------------------------------------------

template<transport_protocol_t transport_protocol>
class socket_sync : public generic_socket
{
public:
    inline socket_sync() throw() { configure(ipv4, transport_protocol, blocking); }
};

//------------------------------------------------------------------------------

template<transport_protocol_t transport_protocol>
class ipv6socket : public generic_socket
{
public:
    inline ipv6socket() throw() { configure(ipv6, transport_protocol, nonblocking); }
};

//------------------------------------------------------------------------------

template<transport_protocol_t transport_protocol>
class ipv6socket_sync : public generic_socket
{
public:
    inline ipv6socket_sync() throw() { configure(ipv6, transport_protocol, blocking); }
};

//------------------------------------------------------------------------------
}
