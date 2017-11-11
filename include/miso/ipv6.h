// (C) unresolved-external@singu-lair.com released under the MIT license (see LICENSE)

#pragma once

#include "miso/utility.h"

struct in6_addr;

namespace miso {
//------------------------------------------------------------------------------

class ipv6_address : public ip_address
{
public:
    virtual ~ipv6_address();
    virtual int get_family() const override;
    virtual int get_type() const override;
    virtual std::string to_string() const override;

    inline ipv6_address() {}
    ipv6_address(const char*);
    inline bool is_valid() const { return impl != nullptr; }
    bool retrieve_platform_implementation(void* buffer, size_t buffer_size) const;

    ipv6_address(const ipv6_address&);
    ipv6_address(ipv6_address&&);
    ipv6_address& operator=(const ipv6_address&);
    ipv6_address& operator=(ipv6_address&&);

    static ipv6_address any();

private:
    virtual bool set_raw(void* addr, size_t addrlen) override;

private:
    struct ::in6_addr* impl = nullptr;
};

//------------------------------------------------------------------------------
}
