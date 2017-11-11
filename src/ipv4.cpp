// (C) unresolved-external@singu-lair.com released under the MIT license (see LICENSE)

#include "miso/ipv4.h"
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

ipv4_address::ipv4_address(const char* ip_str)
{
    impl = new struct ::in_addr();
    if (_inet_pton(AF_INET, ip_str, impl) <= 0)
    {
        delete impl;
        impl = nullptr;
    }
}

//------------------------------------------------------------------------------

ipv4_address::~ipv4_address()
{
    delete impl;
}

//------------------------------------------------------------------------------

int ipv4_address::get_family() const
{
    return (int)AF_INET;
}

//------------------------------------------------------------------------------

int ipv4_address::get_type() const
{
    return (int)ipv4;
}

//------------------------------------------------------------------------------

std::string ipv4_address::to_string() const
{
    char str[INET_ADDRSTRLEN];
    return (inet_ntop(AF_INET, impl, str, INET_ADDRSTRLEN)) ? std::string(str) : std::string();
}

//------------------------------------------------------------------------------

bool ipv4_address::retrieve_platform_implementation(void* buffer, size_t buffer_size) const
{
    if (!impl)
        return false;

    if (buffer_size < sizeof(*impl))
        return false;

    memcpy(buffer, impl, sizeof(*impl));
    return true;
}

//------------------------------------------------------------------------------

ipv4_address::ipv4_address(const ipv4_address& other)
    : impl(other.impl ? new struct ::in_addr(*other.impl) : nullptr)
{}

//------------------------------------------------------------------------------

ipv4_address::ipv4_address(ipv4_address&& from)
    : impl(from.impl)
{
    from.impl = nullptr;
}

//------------------------------------------------------------------------------

ipv4_address& ipv4_address::operator=(const ipv4_address& other)
{
    delete impl;
    impl = other.impl ? new struct ::in_addr(*other.impl) : nullptr;
    return *this;
}

//------------------------------------------------------------------------------

ipv4_address& ipv4_address::operator=(ipv4_address&& from)
{
    delete impl;
    impl = from.impl;
    from.impl = nullptr;
    return *this;
}

//------------------------------------------------------------------------------

ipv4_address ipv4_address::any()
{
    ipv4_address address;
    address.impl = new struct ::in_addr();
    address.impl->s_addr = htonl(INADDR_ANY);
    return address;
}

//------------------------------------------------------------------------------

bool ipv4_address::set_raw(void* addr, size_t addrlen)
{
    if (((::sockaddr_storage*)addr)->ss_family != AF_INET)
        return false;
    delete impl;
    impl = new struct ::in_addr(((::sockaddr_in*)addr)->sin_addr);
    return true;
}

//------------------------------------------------------------------------------
}
