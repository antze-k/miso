// (C) unresolved-external@singu-lair.com released under the MIT license (see LICENSE)

#pragma once

#include "miso/utility.h"

struct in_addr;

namespace miso {
//------------------------------------------------------------------------------

class ipv4_address : public ip_address
{
public:
    virtual ~ipv4_address();
    virtual unsigned int get_family() const override;
    virtual int get_type() const override;

    inline ipv4_address() {}
    ipv4_address(const char*);
    inline bool is_valid() const { return impl != nullptr; }
    bool retrieve_platform_implementation(void* buffer, size_t buffer_size) const;

    ipv4_address(const ipv4_address&);
    ipv4_address(ipv4_address&&);
    ipv4_address& operator=(const ipv4_address&);
    ipv4_address& operator=(ipv4_address&&);

    static ipv4_address any();

private:
    struct ::in_addr* impl = nullptr;
};

//------------------------------------------------------------------------------
}
