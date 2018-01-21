// (C) unresolved-external@singu-lair.com released under the MIT license (see LICENSE)

#pragma once

#include <string>

namespace miso {
//------------------------------------------------------------------------------

enum internet_protocol_t
{
    ipv4,
    ipv6,
};

//------------------------------------------------------------------------------

enum transport_protocol_t
{
    tcp,
    udp,
};

//------------------------------------------------------------------------------

enum socket_mode_t
{
    blocking,
    nonblocking,
};

//------------------------------------------------------------------------------

enum socket_error_t
{
    error_none,
    error_bad_alloc,
    error_not_configured,
    error_not_connected,
    error_invalid_usage,
    error_internal,
    error_socket,
};

//------------------------------------------------------------------------------

class ip_address
{
public:
    ip_address() throw();
    ~ip_address() throw();

    ip_address(const ip_address&) throw();
    ip_address(ip_address&&) throw();
    ip_address& operator=(const ip_address&) throw();
    ip_address& operator=(ip_address&&) throw();

    inline int get_family() const throw() { return af; }
    internet_protocol_t get_type() const throw();
    std::string to_string() const throw();

    inline bool is_valid() const { return ia != nullptr; }

private:
    ip_address(int af, const char*) throw();
    ip_address(int af, const std::string&) throw();
    ip_address(int af, const void* inx_addr, size_t addrlen) throw();
    bool set_storage(const void* sockaddr, size_t addrlen) throw();
    bool set_raw(int af, const void* inx_addr, size_t addrlen) throw();
    bool get_raw(void* inx_addr, size_t addrlen) const throw();

    friend class generic_socket;
    friend struct ipv4_address;
    friend struct ipv6_address;
    friend struct dns;

private:
    int af;
    void* ia = nullptr;
};

//------------------------------------------------------------------------------
}
