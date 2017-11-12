// (C) unresolved-external@singu-lair.com released under the MIT license (see LICENSE)

#pragma once

#include "miso/socket/socketdef.h"

namespace miso {
//------------------------------------------------------------------------------

struct ipv4_address
{
    static ip_address create(const char*);
    static ip_address create(const std::string&);
    static ip_address any();
};

//------------------------------------------------------------------------------
}
