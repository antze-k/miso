// (C) unresolved-external@singu-lair.com released under the MIT license (see LICENSE)

#pragma once

#include "miso/socket/socketdef.h"

#include <stddef.h>
#include <string.h>

#if defined(WIN32)
    #include <WinSock2.h>
    #include <ws2ipdef.h>
    #include <ws2tcpip.h>
    using internal_socket_t = SOCKET;
    using socklen_t = int;
    #define _SOCKERROR()    WSAGetLastError()
    #define _EINPROGRESS    WSAEINPROGRESS
    #define _EWOULDBLOCK    WSAEWOULDBLOCK
    #define _EAGAIN         WSAEWOULDBLOCK
#else
    #include <arpa/inet.h>
    #include <errno.h>
    #include <fcntl.h>
    #include <netdb.h>
    #include <netinet/in.h>
    #include <signal.h>
    #include <string.h>
    #include <sys/socket.h>
    #include <sys/time.h>
    #include <sys/types.h>
    #include <time.h>
    #include <unistd.h>
    using internal_socket_t = int;
    #define INVALID_SOCKET (int)(~0)
    #define _SOCKERROR()    errno
    #define _EINPROGRESS    EINPROGRESS
    #define _EWOULDBLOCK    EWOULDBLOCK
    #define _EAGAIN         EAGAIN
#endif

namespace miso { namespace tools {
//------------------------------------------------------------------------------

inline int to_api_family(internet_protocol_t internet_protocol)
{
    switch (internet_protocol)
    {
        case ipv4: return AF_INET;
        case ipv6: return AF_INET6;
        default: return AF_UNSPEC;
    }
}

//------------------------------------------------------------------------------

inline internet_protocol_t from_api_family(int af)
{
    switch (af)
    {
        case AF_INET: return ipv4;
        case AF_INET6: return ipv6;
        default: return (internet_protocol_t)-1;
    }
}

//------------------------------------------------------------------------------

inline int to_socket_type(transport_protocol_t transport_protocol)
{
    switch (transport_protocol)
    {
        case tcp: return SOCK_STREAM;
        case udp: return SOCK_DGRAM;
        default: return -1;
    }
}

//------------------------------------------------------------------------------

inline transport_protocol_t from_socket_type(int type)
{
    switch (type)
    {
        case SOCK_STREAM: return tcp;
        case SOCK_DGRAM: return udp;
        default: return (transport_protocol_t)-1;
    }
}

//------------------------------------------------------------------------------

inline int to_protocol(transport_protocol_t transport_protocol)
{
    switch (transport_protocol)
    {
        case tcp: return IPPROTO_TCP;
        case udp: return IPPROTO_UDP;
        default: return -1;
    }
}

//------------------------------------------------------------------------------

inline transport_protocol_t from_protocol(int proto)
{
    switch (proto)
    {
        case IPPROTO_TCP: return tcp;
        case IPPROTO_UDP: return udp;
        default: return (transport_protocol_t)-1;
    }
}

//------------------------------------------------------------------------------
} }
