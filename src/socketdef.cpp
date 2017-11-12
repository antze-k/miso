// (C) unresolved-external@singu-lair.com released under the MIT license (see LICENSE)

#include "miso/socket/socketdef.h"

#include "socket_tools.h"

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

namespace {
//------------------------------------------------------------------------------

inline void delete_ia_fast(int& af, void* ia) throw()
{
    switch (af)
    {
        case AF_INET: delete (struct ::in_addr*)ia; break;
        case AF_INET6: delete (struct ::in6_addr*)ia; break;
        default: break;
    }
    af = AF_UNSPEC;
}

//------------------------------------------------------------------------------

inline void delete_ia(int& af, void*& ia) throw()
{
    switch (af)
    {
        case AF_INET: delete (struct ::in_addr*)ia; break;
        case AF_INET6: delete (struct ::in6_addr*)ia; break;
        default: break;
    }
    af = AF_UNSPEC;
    ia = nullptr;
}

//------------------------------------------------------------------------------

template<typename inx_addr>
inline void set_ia(void*& ia, const void* other_ia) throw()
{
    ia = new inx_addr(*(inx_addr*)other_ia);
}

//------------------------------------------------------------------------------

inline void set_ia(int af, void*& ia, const void* other_ia) throw()
{
    switch (af)
    {
        case AF_INET: set_ia<struct ::in_addr>(ia, other_ia); break;
        case AF_INET6: set_ia<struct ::in6_addr>(ia, other_ia); break;
        default: ia = nullptr; break;
    }
}

//------------------------------------------------------------------------------
}

namespace miso {
//------------------------------------------------------------------------------

ip_address::ip_address() throw()
    : af(AF_UNSPEC)
    , ia(nullptr)
{}

//------------------------------------------------------------------------------

ip_address::~ip_address() throw()
{
    delete_ia_fast(af, ia);
}

//------------------------------------------------------------------------------

ip_address::ip_address(const ip_address& other) throw()
    : af(other.af)
{
    set_ia(af, ia, other.ia);
}

//------------------------------------------------------------------------------

ip_address::ip_address(ip_address&& from) throw()
    : af(from.af)
    , ia(from.ia)
{
    from.ia = nullptr;
}

//------------------------------------------------------------------------------

ip_address& ip_address::operator=(const ip_address& other) throw()
{
    delete_ia(af, ia);
    af = other.af;
    set_ia(af, ia, other.ia);
    return *this;
}

//------------------------------------------------------------------------------

ip_address& ip_address::operator=(ip_address&& from) throw()
{
    delete_ia(af, ia);
    af = from.af;
    ia = from.ia;
    from.ia = nullptr;
    return *this;
}

//------------------------------------------------------------------------------

internet_protocol_t ip_address::get_type() const throw()
{
    return tools::from_api_family(af);
}

//------------------------------------------------------------------------------

std::string ip_address::to_string() const throw()
{
    switch (af)
    {
        case AF_INET:
        {
            char str[INET_ADDRSTRLEN];
            return (inet_ntop(AF_INET, ia, str, INET_ADDRSTRLEN)) ? std::string(str) : std::string();
        }

        case AF_INET6:
        {
            char str[INET6_ADDRSTRLEN];
            return (inet_ntop(AF_INET6, ia, str, INET6_ADDRSTRLEN)) ? std::string(str) : std::string();
        }

        default: break;
    }

    return {};
}

//------------------------------------------------------------------------------

ip_address::ip_address(int af, const char* ip_str) throw()
    : af(af)
{
    if (!ip_str) af = AF_UNSPEC;

    switch (af)
    {
        case AF_INET: ia = new struct ::in_addr(); break;
        case AF_INET6: ia = new struct ::in6_addr(); break;
        default: ia = nullptr; return;
    }

    if (_inet_pton(af, ip_str, ia) <= 0)
        delete_ia(af, ia);
}

//------------------------------------------------------------------------------

ip_address::ip_address(int af, const std::string& ip_str) throw()
{
    switch (af)
    {
        case AF_INET: ia = new struct ::in_addr(); break;
        case AF_INET6: ia = new struct ::in6_addr(); break;
        default: ia = nullptr; return;
    }

    if (_inet_pton(af, ip_str.c_str(), ia) <= 0)
        delete_ia(af, ia);
}

//------------------------------------------------------------------------------

ip_address::ip_address(int af, const void* inx_addr, size_t addrlen) throw()
    : af(af)
    , ia(nullptr)
{
    set_raw(af, inx_addr, addrlen);
}

//------------------------------------------------------------------------------

bool ip_address::set_storage(const void* sockaddr, size_t addrlen) throw()
{
    const int other_af = ((struct ::sockaddr_storage*)sockaddr)->ss_family;
    switch (other_af)
    {
        default: break;
        case AF_INET: return set_raw(AF_INET, &((struct ::sockaddr_in*)sockaddr)->sin_addr, sizeof(in_addr));
        case AF_INET6: return set_raw(AF_INET, &((struct ::sockaddr_in6*)sockaddr)->sin6_addr, sizeof(in6_addr));
    }
    return false;
}

//------------------------------------------------------------------------------

bool ip_address::set_raw(int other_af, const void* addr, size_t addrlen) throw()
{
    switch (other_af)
    {
        default: return false;
        case AF_INET:
            delete_ia(af, ia);
            if (addrlen < sizeof(struct ::in_addr)) return false;
            set_ia<struct ::in_addr>(ia, addr);
            break;
        case AF_INET6:
            delete_ia(af, ia);
            if (addrlen < sizeof(struct ::in6_addr)) return false;
            set_ia<struct ::in6_addr>(ia, addr);
            break;
    }

    af = other_af;
    return true;
}

//------------------------------------------------------------------------------

bool ip_address::get_raw(void* addr, size_t addrlen) const throw()
{
    if (!ia)
        return false;

    switch (af)
    {
        default: return false;
        case AF_INET:
            if (addrlen < sizeof(struct ::in_addr)) return false;
            memcpy(addr, ia, sizeof(struct ::in_addr));
            break;
        case AF_INET6:
            if (addrlen < sizeof(struct ::in6_addr)) return false;
            memcpy(addr, ia, sizeof(struct ::in6_addr));
            break;
    }

    return true;
}

//------------------------------------------------------------------------------
}
