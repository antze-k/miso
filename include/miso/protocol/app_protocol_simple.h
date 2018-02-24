// (C) unresolved-external@singu-lair.com released under the MIT license (see LICENSE)

#pragma once

#include "miso/protocol/app_protocol.h"

#include <stddef.h>

namespace miso {
//------------------------------------------------------------------------------

class app_protocol_simple : public app_protocol
{
public:
    virtual bool push_out_message(const std::vector<std::uint8_t>& message, char* buf, size_t size, size_t& used) override;
    virtual bool pop_in_message(const char* buf, size_t size, size_t& used, std::vector<std::uint8_t>& message) override;
};

//------------------------------------------------------------------------------
}
