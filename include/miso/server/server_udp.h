// (C) unresolved-external@singu-lair.com released under the MIT license (see LICENSE)

#pragma once

#include "miso/utility.h"

#include <cstdint>
#include <string>
#include <utility>

namespace miso {
//------------------------------------------------------------------------------

class generic_socket;

//------------------------------------------------------------------------------

class server_udp final : public noncopyable, public nonmovable
{
public:
    bool open(std::uint16_t port);
    bool open(generic_socket&&);
    void close();

public:
    void update();

public:
    using client_id = unsigned int;

public:
    bool send(client_id id, const void* data, std::uint16_t size);

public:
        // persistent between update calls
    bool is_up() const;

    size_t get_client_count() const;
    client_id get_client(size_t index) const;

    size_t get_message_count() const;
    const std::pair<client_id, std::string>& get_message(size_t index) const;

public:
    void disconnect_user(client_id);

public:
    inline ~server_udp() { close(); }
};

//------------------------------------------------------------------------------
}
