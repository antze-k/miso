// (C) unresolved-external@singu-lair.com released under the MIT license (see LICENSE)

#include "miso/dns/dns.h"
#include "miso/socket/ipv4.h"
#include "miso/socket/ipv6.h"

#include "socket_tools.h"

#include <cstring>

namespace miso {
//------------------------------------------------------------------------------

ip_address dns::resolve_ipv4_sync(const char* host_str)
{
    if (!host_str || !host_str[0])
        return ip_address();

    struct ::addrinfo hints;
    ::memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = AF_INET;

    struct ::addrinfo* result;
    if (0 != getaddrinfo(host_str, nullptr, nullptr, &result))
        return ip_address();

    if (!result || result->ai_family != AF_INET)
    {
        freeaddrinfo(result);
        return ip_address();
    }

    ip_address address(AF_INET, &((struct sockaddr_in*)result->ai_addr)->sin_addr, sizeof(::in_addr));
    freeaddrinfo(result);
    return address;
}

//------------------------------------------------------------------------------

ip_address dns::resolve_ipv6_sync(const char* host_str)
{
    if (!host_str || !host_str[0])
        return ip_address();

    struct ::addrinfo hints;
    ::memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = AF_INET6;

    struct ::addrinfo* result;
    if (0 != getaddrinfo(host_str, nullptr, nullptr, &result))
        return ip_address();

    if (!result || result->ai_family != AF_INET6)
    {
        freeaddrinfo(result);
        return ip_address();
    }

    ip_address address(AF_INET6, &((struct sockaddr_in6*)result->ai_addr)->sin6_addr, sizeof(::in6_addr));
    freeaddrinfo(result);
    return address;
}

//------------------------------------------------------------------------------

bool dns::resolve_sync(const char* host_str, std::function<void(ip_address&&)> receiver)
{
    if (!host_str || !host_str[0])
        return false;

    struct ::addrinfo* result;
    if (0 != getaddrinfo(host_str, nullptr, nullptr, &result))
        return false;

    if (!result)
    {
        freeaddrinfo(result);
        return false;
    }

    bool found = false;
    for (struct ::addrinfo* ai = result; ai != nullptr; ai = ai->ai_next)
    {
        switch (ai->ai_family)
        {
            default: continue;
            case AF_INET:
                receiver(ip_address(AF_INET, &((struct sockaddr_in*)result->ai_addr)->sin_addr, sizeof(::in_addr)));
                found = true;
                break;

            case AF_INET6:
                receiver(ip_address(AF_INET6, &((struct sockaddr_in6*)result->ai_addr)->sin6_addr, sizeof(::in6_addr)));
                found = true;
                break;
        }
    }

    freeaddrinfo(result);
    return found;
}

//------------------------------------------------------------------------------
}
