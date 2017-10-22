// (C) unresolved-external@singu-lair.com released under the MIT license (see LICENSE)

#include "miso/platform.h"
#include <new>

#if defined(WIN32)
    #include <WinSock2.h>
#endif

namespace miso {
//------------------------------------------------------------------------------

bool sockets_init()
{
#if defined(WIN32)
    WSADATA wsa_data;
    WORD version_requested = MAKEWORD(1, 1);
    if (::WSAStartup(version_requested, &wsa_data) != 0)
        return false;
#endif

    return true;
}

//------------------------------------------------------------------------------

bool sockets_shutdown()
{
#if defined(WIN32)
    WSACleanup();
#endif

    return true;
}

//------------------------------------------------------------------------------
}
