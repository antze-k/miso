// (C) unresolved-external@singu-lair.com released under the MIT license (see LICENSE)

#pragma once

#include <cstdint>
#include <string>

namespace miso {
//------------------------------------------------------------------------------

class app_protocol
{
public:
    virtual bool push_out_message(const std::string& message, char* buf, size_t size, size_t& used) = 0;
    virtual bool pop_in_message(const char* buf, size_t size, size_t& used, std::string& message) = 0;
};

//------------------------------------------------------------------------------
}
