// (C) unresolved-external@singu-lair.com released under the MIT license (see LICENSE)

#pragma once

#if defined(WIN32)
    #pragma comment(lib, "ws2_32.lib")
#endif

namespace miso {
//------------------------------------------------------------------------------

bool sockets_init();
bool sockets_shutdown();

//------------------------------------------------------------------------------
}
