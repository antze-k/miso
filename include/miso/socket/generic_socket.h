// (C) unresolved-external@singu-lair.com released under the MIT license (see LICENSE)

#pragma once

#include "miso/utility.h"
#include "miso/socket/socketdef.h"

namespace miso {
//------------------------------------------------------------------------------

class generic_socket : public noncopyable
{
public:
    generic_socket() = default;
    generic_socket(generic_socket&&);
    generic_socket& operator=(generic_socket&&);

public:
    bool configure(internet_protocol_t, transport_protocol_t, socket_mode_t, bool reuse_addr = false);
    void close();

    bool connect(const ip_address&, unsigned short port);
    bool listen(const ip_address&, unsigned short port);
    bool listen(unsigned short port);
    bool proceed(int wait_timeout_ms = 0);
    bool accept(generic_socket&);

    internet_protocol_t get_internet_protocol() const;
    transport_protocol_t get_transport_protocol() const;

    bool export_peer(ip_address*, unsigned short*) const;

    int send(const void*, size_t);
    int recv(void*, size_t);

public:
    struct status
    {
        bool up = false;
        socket_error_t error = error_none;
        int platform_errno = 0;
    };

public:
    bool is_configured() const;
    bool is_open() const;
    const status& get_status() const;

    bool retrieve_platform_implementation(void* buffer, size_t buffer_size) const;

private:
    struct node;
    node* m_node = nullptr;
    status m_status;

    void _destroy_node();
    bool _success();
    bool _report(socket_error_t);
    bool _fail(socket_error_t, int platform_errno = -1);

public:
    ~generic_socket();
};

//------------------------------------------------------------------------------
}
