// (C) unresolved-external@singu-lair.com released under the MIT license (see LICENSE)

#include "miso/socket/generic_socket.h"
#include "miso/ipv4.h"
#include "miso/ipv6.h"
#include <new>

#if defined(WIN32) && !defined(MISO_DISABLE_AUTO_WINSOCK)
    #include "miso/platform.h"
#endif

namespace {
//------------------------------------------------------------------------------

#include <stddef.h>
#include <string.h>

#if defined(WIN32)
    #include <WinSock2.h>
    #include <ws2ipdef.h>
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

//------------------------------------------------------------------------------

inline int to_api_family(::miso::internet_protocol_t internet_protocol)
{
    switch (internet_protocol)
    {
        case ::miso::ipv4: return AF_INET;
        case ::miso::ipv6: return AF_INET6;
        default: return AF_UNSPEC;
    }
}

//------------------------------------------------------------------------------

inline int to_socket_type(::miso::transport_protocol_t transport_protocol)
{
    switch (transport_protocol)
    {
        case ::miso::tcp: return SOCK_STREAM;
        case ::miso::udp: return SOCK_DGRAM;
        default: return -1;
    }
}

//------------------------------------------------------------------------------

inline int to_protocol(::miso::transport_protocol_t transport_protocol)
{
    switch (transport_protocol)
    {
        case ::miso::tcp: return IPPROTO_TCP;
        case ::miso::udp: return IPPROTO_UDP;
        default: return -1;
    }
}
//------------------------------------------------------------------------------
}

namespace miso {
//------------------------------------------------------------------------------

struct generic_socket::node
{
    struct config_t
    {
        int af;
        int type;
        int proto;
        socket_mode_t socket_mode;
        bool reuse_addr = false;
    };

    config_t config;
    internal_socket_t sock = INVALID_SOCKET;

    int open()
    {
        close();

            // create

        sock = ::socket(config.af, config.type, config.proto);
        if (sock == INVALID_SOCKET || sock <= 0)
        {
            const int error = _SOCKERROR();

            #if defined(WIN32) && !defined(MISO_DISABLE_AUTO_WINSOCK)

            if (error == WSANOTINITIALISED)
            {
                if (!sockets_init())
                    return close(), WSANOTINITIALISED;

                sock = ::socket(config.af, config.type, config.proto);
                if (sock == INVALID_SOCKET || sock <= 0)
                {
                    const int error2 = _SOCKERROR();
                    return close(), error2;
                }
            }
            else

            #endif
                return close(), error;
        }

            // set nonblocking mode

        if (config.socket_mode == nonblocking)
        {
        #if defined(WIN32)
            unsigned long yes = 1;
            if (::ioctlsocket(sock, FIONBIO, &yes) == SOCKET_ERROR)
        #else
            int flags = ::fcntl(sock, F_GETFL, 0);
            if (flags >= 0)
            {
                flags = (flags | O_NONBLOCK);
                flags = ::fcntl(sock, F_SETFL, flags);
            }
            if (flags < 0)
        #endif
            {
                const int error = _SOCKERROR();
                return close(), error;
            }
        }

            // set reuseaddr

        if (config.reuse_addr)
        {
        #if defined(WIN32)
            u_long yes = 1;
            if (::setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, (char*)&yes, sizeof(yes)) < 0)
        #else
            int yes = 1;
            if (::setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, (void*)&yes, sizeof(yes)) < 0)
        #endif
            {
                const int error = _SOCKERROR();
                return close(), error;
            }
        }

            // set nosigpipe

        #if defined(__APPLE__)
        {
            int yes = 1;
            if (::setsockopt(sock, SOL_SOCKET, SO_NOSIGPIPE, (void*)&yes, sizeof(yes)) < 0)
            {
                const int error = _SOCKERROR();
                return close(), error;
            }
        }
        #endif

        return 0;
    }

    void close() throw()
    {
        if (sock == INVALID_SOCKET)
            return;

        #if defined(WIN32)
            ::closesocket(sock);
        #else
            ::close(sock);
        #endif

        sock = INVALID_SOCKET;
    }

    inline ~node() throw() { close(); }

    inline bool is_open() const throw() { return sock != INVALID_SOCKET; }

    inline int get_socket_error() const throw()
    {
        int error;
        socklen_t error_size = (socklen_t)sizeof(error);
        if (::getsockopt(sock, SOL_SOCKET, SO_ERROR, (char*)&error, &error_size) < 0)
            return -1;
        return error;
    }
};

//------------------------------------------------------------------------------

generic_socket::generic_socket(generic_socket&& from)
    : m_node(from.m_node)
    , m_status(from.m_status)
{
    from.m_node = nullptr;
}

//------------------------------------------------------------------------------

generic_socket& generic_socket::operator=(generic_socket&& from)
{
    delete m_node;
    m_node = from.m_node;
    m_status = from.m_status;
    from.m_node = nullptr;
    return *this;
}

//------------------------------------------------------------------------------

bool generic_socket::configure(internet_protocol_t internet_protocol, transport_protocol_t transport_protocol,
        socket_mode_t socket_mode, bool reuse_addr)
{
    _destroy_node();
    m_status = status();

    try
    {
        m_node = new node();
    }
    catch (std::bad_alloc&)
    {
        m_node = nullptr;
        m_status.error = error_bad_alloc;
        return false;
    }
    if (m_node == nullptr)
    {
        m_status.error = error_bad_alloc;
        return false;
    }

    m_node->config.af = to_api_family(internet_protocol);
    m_node->config.type = to_socket_type(transport_protocol);
    m_node->config.proto = to_protocol(transport_protocol);
    m_node->config.socket_mode = socket_mode;
    m_node->config.reuse_addr = reuse_addr;

    return true;
}

//------------------------------------------------------------------------------

void generic_socket::close()
{
    if (m_node)
    {
        m_node->close();
        m_status = status();
    }
}

//------------------------------------------------------------------------------

bool generic_socket::connect(const ip_address& address, unsigned short port)
{
    if (!m_node)
        return _report(error_not_configured);

    close();

    switch (address.get_family())
    {
        case AF_INET:
        case AF_INET6:
            if (m_node->config.af != address.get_family())
                return _report(error_invalid_usage);
            break;
        default:
            return _report(error_invalid_usage);
    }

    const int err3 = m_node->open();
    if (err3 != 0)
        return _fail(error_socket, err3);

    bool connected = false;

    switch (address.get_family())
    {
        case AF_INET:
        {
            struct sockaddr_in sa = {0};
            sa.sin_family   = static_cast<decltype(sa.sin_family)>((unsigned int)address.get_family());
            sa.sin_port     = htons(port);
            static_cast<const ipv4_address&>(address)
                .retrieve_platform_implementation(&sa.sin_addr, sizeof(sa.sin_addr));

            connected = (::connect(m_node->sock, (struct sockaddr*)&sa, sizeof(sa)) >= 0);
        }
        break;

        case AF_INET6:
        {
            struct sockaddr_in6 sa = {0};
            sa.sin6_family  = static_cast<decltype(sa.sin6_family)>((unsigned int)address.get_family());
            sa.sin6_port    = htons(port);
            static_cast<const ipv6_address&>(address)
                .retrieve_platform_implementation(&sa.sin6_addr, sizeof(sa.sin6_addr));

            connected = (::connect(m_node->sock, (struct sockaddr*)&sa, sizeof(sa)) >= 0);
        }
        break;

        default:
            return _fail(error_internal);
    }

    if (!connected)
    {
        const int err4 = _SOCKERROR();
        if (err4 == _EINPROGRESS || err4 == _EWOULDBLOCK || err4 == _EAGAIN)
            return _report(error_none);  // connecting

        return _fail(error_socket, err4);  // failed to connect
    }

    const int errx = m_node->get_socket_error();
    if (errx != 0)
        return _fail(error_socket, errx);

    m_status.up = true;
    return _success();
}

//------------------------------------------------------------------------------

bool generic_socket::listen(const ip_address& address, unsigned short port)
{
    if (!m_node)
        return _report(error_not_configured);

    close();

    switch (address.get_family())
    {
        case AF_INET:
        case AF_INET6:
            if (m_node->config.af != address.get_family())
                return _report(error_invalid_usage);
            break;
        default:
            return _report(error_invalid_usage);
    }

    const int err3 = m_node->open();
    if (err3 != 0)
        return _fail(error_socket, err3);

    bool bound = false;

    switch (address.get_family())
    {
        case AF_INET:
        {
            struct sockaddr_in sa = {0};
            sa.sin_family   = static_cast<decltype(sa.sin_family)>((unsigned int)address.get_family());
            sa.sin_port     = htons(port);
            static_cast<const ipv4_address&>(address)
                .retrieve_platform_implementation(&sa.sin_addr, sizeof(sa.sin_addr));

            bound = (::bind(m_node->sock, (struct sockaddr*)&sa, sizeof(sa)) == 0);
        }
        break;

        case AF_INET6:
        {
            struct sockaddr_in6 sa = {0};
            sa.sin6_family  = static_cast<decltype(sa.sin6_family)>((unsigned int)address.get_family());
            sa.sin6_port    = htons(port);
            static_cast<const ipv6_address&>(address)
                .retrieve_platform_implementation(&sa.sin6_addr, sizeof(sa.sin6_addr));

            bound = (::bind(m_node->sock, (struct sockaddr*)&sa, sizeof(sa)) == 0);
        }
        break;

        default:
            return _fail(error_internal);
    }

    if (!bound)
    {
        const int err4 = _SOCKERROR();
        if (err4 == _EINPROGRESS || err4 == _EWOULDBLOCK || err4 == _EAGAIN)
            return _report(error_none);  // connecting

        return _fail(error_socket, err4);  // failed to bind
    }

    if (::listen(m_node->sock, SOMAXCONN) != 0)
    {
        const int err5 = _SOCKERROR();
        if (err5 == _EINPROGRESS || err5 == _EWOULDBLOCK || err5 == _EAGAIN)
            return _report(error_none);  // connecting

        return _fail(error_socket, err5);  // failed to listen
    }

    const int errx = m_node->get_socket_error();
    if (errx != 0)
        return _fail(error_socket, errx);

    m_status.up = true;
    return _success();
}

//------------------------------------------------------------------------------

bool generic_socket::listen(unsigned short port)
{
    if (!m_node)
        return _report(error_not_configured);

    switch (m_node->config.af)
    {
        case AF_INET:
            return this->listen(ipv4_address::any(), port);
        case AF_INET6:
            return this->listen(ipv6_address::any(), port);
        default:
            break;
    }

    return _report(error_invalid_usage);
}

//------------------------------------------------------------------------------

bool generic_socket::proceed(int wait_timeout_ms)
{
    if (!m_node)
        return _report(error_not_configured);

    if (!m_node->is_open())
        return _report(error_invalid_usage);

    if (m_status.up)
        return _success();

    if (m_node->config.socket_mode == blocking)
        return _report(error_not_connected);

    fd_set wfds;
    FD_ZERO(&wfds);
    FD_SET(m_node->sock, &wfds);

    int selected = 0;
    if (wait_timeout_ms < 0)  // as long as possible
    {
        #if defined(WIN32)
            selected = select(0, 0, &wfds, 0, 0);
        #else
            selected = select(((int)m_node->sock) + 1, 0, &wfds, 0, 0);
        #endif
    }
    else
    {
        struct timeval wait_time = {0};
        wait_time.tv_sec    = wait_timeout_ms / 1000;
        wait_time.tv_usec   = wait_timeout_ms % 1000;

        #if defined(WIN32)
            selected = select(0, 0, &wfds, 0, &wait_time);
        #else
            selected = select(((int)m_node->sock) + 1, 0, &wfds, 0, &wait_time);
        #endif
    }

    if (selected < 0)
    {
        const int err1 = _SOCKERROR();
        if (!(err1 == _EINPROGRESS || err1 == _EWOULDBLOCK || err1 == _EAGAIN))
            return _fail(error_socket, err1);  // failed_to_connect
    }

    const int err2 = m_node->get_socket_error();
    if (err2 != 0)
        return _fail(error_socket, err2);

    if (!FD_ISSET(m_node->sock, &wfds))
        return _report(error_none);  // connecting

    m_status.up = true;
    return _success();  // connected
}

//------------------------------------------------------------------------------

bool generic_socket::accept(generic_socket& accepted)
{
    accepted._destroy_node();

    if (!m_node)
        return _report(error_not_configured);
    if (m_node->config.type != SOCK_STREAM)
        return _report(error_invalid_usage);
    if (!m_node->is_open())
        return _report(error_invalid_usage);

    if (m_node->config.socket_mode == blocking)
    {
        fd_set wfds;
        FD_ZERO(&wfds);
        FD_SET(m_node->sock, &wfds);

        struct timeval nulltime = {0};
    #if defined(WIN32)
        const int selected = select(0, 0, &wfds, 0, &nulltime);
    #else
        const int selected = select(((int)m_node->sock) + 1, 0, &wfds, 0, &nulltime);
    #endif
        if (selected < 0)
            return _report(error_internal);
        if (selected == 0)
            return false;
    }

    switch (m_node->config.af)
    {
        default:
            return _report(error_internal);

        case AF_INET:
        {
            struct sockaddr_in addr;
            socklen_t addrlen = sizeof(addr);
            internal_socket_t _accepted = ::accept(m_node->sock, (struct sockaddr*)&addr, &addrlen);
            if (_accepted == INVALID_SOCKET)
            {
                m_status.error = error_socket;
                m_status.platform_errno = _SOCKERROR();
                return false;
            }

            accepted.configure(ipv4, tcp, m_node->config.socket_mode);
            accepted.m_node->sock = _accepted;
        }
        break;

        case AF_INET6:
        {
            struct sockaddr_in6 addr;
            socklen_t addrlen = sizeof(addr);
            internal_socket_t _accepted = ::accept(m_node->sock, (struct sockaddr*)&addr, &addrlen);
            if (_accepted == INVALID_SOCKET)
            {
                m_status.error = error_socket;
                m_status.platform_errno = _SOCKERROR();
                return false;
            }

            accepted.configure(ipv6, tcp, m_node->config.socket_mode);
            accepted.m_node->sock = _accepted;
        }
        break;
    }

    if (!accepted.is_configured())
        return false;

    accepted.m_status.up = true;
    return true;

}

//------------------------------------------------------------------------------

int generic_socket::send(const void* data, size_t size)
{
    if (!m_node)
    {
        _report(error_not_configured);
        return -1;
    }

    if (!m_node->is_open())
    {
        _report(error_not_connected);
        return -1;
    }

    if (size == 0)
        return 0;

    if (!data)
    {
        _report(error_invalid_usage);
        return -1;
    }

    #if defined(WIN32)
        const int sent = ::send(m_node->sock, (const char*)data, (int)size, 0);
    #elif defined(__APPLE__)
        const int sent = (int)::write(m_node->sock, data, size);
    #else
        const int sent = (int)::send(m_node->sock, data, size, MSG_NOSIGNAL);
    #endif

    if (sent < 0)
    {
        _fail(error_socket, _SOCKERROR());
        return -1;
    }

    _success();
    return sent;
}

//------------------------------------------------------------------------------

int generic_socket::recv(void* data, size_t size)
{
    if (!m_node)
    {
        _report(error_not_configured);
        return -1;
    }

    if (!m_node->is_open())
    {
        _report(error_not_connected);
        return -1;
    }

    if (size == 0)
        return 0;

    if (!data)
    {
        _report(error_invalid_usage);
        return -1;
    }

    #if defined(WIN32)
        const int received = ::recv(m_node->sock, (char*)data, (int)size, 0);
    #elif defined(__APPLE__)
        const int received = (int)::read(m_node->sock, data, size);
    #else
        const int received = (int)::recv(m_node->sock, data, size, 0);
    #endif

    if (received == 0)
    {
        _fail(error_internal);
        return -1;
    }

    if (received < 0)
    {
        const int err = _SOCKERROR();
        if (err == _EWOULDBLOCK && err == _EAGAIN)
        {
            _success();
            return 0;
        }
        _fail(error_socket, err);
        return -1;
    }

    _success();
    return received;
}

//------------------------------------------------------------------------------

bool generic_socket::is_configured() const
{
    return m_node != nullptr;
}

//------------------------------------------------------------------------------

bool generic_socket::is_open() const
{
    return m_node != nullptr && m_node->is_open();
}

//------------------------------------------------------------------------------

const generic_socket::status& generic_socket::get_status() const
{
    return m_status;
}

//------------------------------------------------------------------------------

bool generic_socket::retrieve_platform_implementation(void* buffer, size_t buffer_size) const
{
    if (!m_node)
        return false;

    if (buffer_size < sizeof(m_node->sock))
        return false;

    memcpy(buffer, &m_node->sock, sizeof(m_node->sock));
    return true;
}

//------------------------------------------------------------------------------

void generic_socket::_destroy_node()
{
    if (!m_node)
        return;

    delete m_node;
    m_node = nullptr;

    m_status = status();
}

//------------------------------------------------------------------------------

bool generic_socket::_success()
{
    m_status.error = error_none;
    return true;
}

//------------------------------------------------------------------------------

bool generic_socket::_report(socket_error_t error)
{
    m_status.error = error;
    return false;
}

//------------------------------------------------------------------------------

bool generic_socket::_fail(socket_error_t error, int platform_errno)
{
    m_status.error = error;
    m_status.platform_errno = (platform_errno > 0) ? platform_errno : 0;

    m_node->close();
    return false;
}

//------------------------------------------------------------------------------

generic_socket::~generic_socket()
{
    delete m_node;
}

//------------------------------------------------------------------------------
}
