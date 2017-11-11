// (C) unresolved-external@singu-lair.com released under the MIT license (see LICENSE)

#include "miso/ipv6.h"
#include "miso/socket/generic_socket.h"

#if defined(WIN32)
    #include <WinSock2.h>
    #include <WS2tcpip.h>
    #if defined(_USING_V110_SDK71_)
        #pragma warning(disable: 4996)
        namespace { int _inet_pton(int af, const char* src, void* dst)
        {
            struct sockaddr_storage ss;
            int size = sizeof(ss);
            char src_copy[INET6_ADDRSTRLEN + 1];

            ZeroMemory(&ss, sizeof(ss));
            /* stupid non-const API */
            strncpy(src_copy, src, INET6_ADDRSTRLEN + 1);
            src_copy[INET6_ADDRSTRLEN] = 0;

            if (WSAStringToAddressA(src_copy, af, NULL, (struct sockaddr *)&ss, &size) == 0)
            {
                switch (af)
                {
                    case AF_INET:
                        *(struct in_addr*)dst = ((struct sockaddr_in*)&ss)->sin_addr;
                        return 1;
                    case AF_INET6:
                        *(struct in6_addr*)dst = ((struct sockaddr_in6*)&ss)->sin6_addr;
                        return 1;
                }
            }
            return 0;
        } }
    #else
        #define _inet_pton InetPtonA
    #endif
#else
    #include <netinet/ip.h>
    #include <arpa/inet.h>

    #define _inet_pton inet_pton
#endif

#include <cstring>

namespace miso {
//------------------------------------------------------------------------------

ipv6_address::ipv6_address(const char* ip_str)
{
    impl = new struct ::in6_addr();
    if (_inet_pton(AF_INET6, ip_str, impl) <= 0)
    {
        delete impl;
        impl = nullptr;
    }
}

//------------------------------------------------------------------------------

ipv6_address::~ipv6_address()
{
    delete impl;
}

//------------------------------------------------------------------------------

int ipv6_address::get_family() const
{
    return (int)AF_INET6;
}

//------------------------------------------------------------------------------

int ipv6_address::get_type() const
{
    return (int)ipv6;
}

//------------------------------------------------------------------------------

std::string ipv6_address::to_string() const
{
    char str[INET6_ADDRSTRLEN];
    return (inet_ntop(AF_INET6, impl, str, INET6_ADDRSTRLEN)) ? std::string(str) : std::string();
}

//------------------------------------------------------------------------------

bool ipv6_address::retrieve_platform_implementation(void* buffer, size_t buffer_size) const
{
    if (!impl)
        return false;

    if (buffer_size < sizeof(*impl))
        return false;

    memcpy(buffer, impl, sizeof(*impl));
    return true;
}

//------------------------------------------------------------------------------

ipv6_address::ipv6_address(const ipv6_address& other)
    : impl(other.impl ? new struct ::in6_addr(*other.impl) : nullptr)
{}

//------------------------------------------------------------------------------

ipv6_address::ipv6_address(ipv6_address&& from)
    : impl(from.impl)
{
    from.impl = nullptr;
}

//------------------------------------------------------------------------------

ipv6_address& ipv6_address::operator=(const ipv6_address& other)
{
    delete impl;
    impl = other.impl ? new struct ::in6_addr(*other.impl) : nullptr;
    return *this;
}

//------------------------------------------------------------------------------

ipv6_address& ipv6_address::operator=(ipv6_address&& from)
{
    delete impl;
    impl = from.impl;
    from.impl = nullptr;
    return *this;
}

//------------------------------------------------------------------------------

ipv6_address ipv6_address::any()
{
    ipv6_address address;
    address.impl = new struct ::in6_addr(::in6addr_any);
    return address;
}

//------------------------------------------------------------------------------

bool ipv6_address::set_raw(void* addr, size_t addrlen)
{
    if (((::sockaddr_storage*)addr)->ss_family != AF_INET6)
        return false;
    delete impl;
    impl = new struct ::in6_addr(((::sockaddr_in6*)addr)->sin6_addr);
    return true;
}

//------------------------------------------------------------------------------
}
