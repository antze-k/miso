// (C) unresolved-external@singu-lair.com released under the MIT license (see LICENSE)

#pragma once

#include "miso/socket/generic_socket.h"

namespace miso {
//------------------------------------------------------------------------------

template<transport_protocol_t transport_protocol> inline generic_socket socket() { generic_socket s; s.configure(ipv4, transport_protocol, nonblocking); return s; }
template<transport_protocol_t transport_protocol> inline generic_socket socket_sync() { generic_socket s; s.configure(ipv4, transport_protocol, blocking); return s; }
template<transport_protocol_t transport_protocol> inline generic_socket ipv6socket() { generic_socket s; s.configure(ipv6, transport_protocol, nonblocking); return s; }
template<transport_protocol_t transport_protocol> inline generic_socket ipv6socket_sync() { generic_socket s; s.configure(ipv6, transport_protocol, blocking); return s; }

//------------------------------------------------------------------------------
}
