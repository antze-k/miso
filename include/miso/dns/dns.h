// (C) unresolved-external@singu-lair.com released under the MIT license (see LICENSE)

#include "miso/socket/socketdef.h"

#include <functional>

namespace miso {
//------------------------------------------------------------------------------

struct dns
{
    static ip_address resolve_ipv4_sync(const char*);
    static ip_address resolve_ipv6_sync(const char*);
    static bool resolve_sync(const char*, std::function<void(ip_address&&)>);
};

//------------------------------------------------------------------------------
}
