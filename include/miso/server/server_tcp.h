// (C) unresolved-external@singu-lair.com released under the MIT license (see LICENSE)

#pragma once

#include "miso/utility.h"
#include "miso/server/server_interface.h"

#include <utility>

namespace miso {
//------------------------------------------------------------------------------

class generic_socket;
class app_protocol;

//------------------------------------------------------------------------------

class server_tcp final : public noncopyable, public nonmovable, public server_interface
{
public:
    struct configuration
    {
        static const size_t default_app_buffer_size = 8192;

        size_t app_input_buffer_size    = default_app_buffer_size;
        size_t app_output_buffer_size   = default_app_buffer_size;
    };

public:
    void set_app_protocol(app_protocol*);

    bool open_ipv4(std::uint16_t port, const configuration& cfg = configuration());
    bool open_ipv6(std::uint16_t port, const configuration& cfg = configuration());
    bool assign(generic_socket&&, const configuration& cfg = configuration());
    void close(bool free_data = false);

public:
    void update();

public:
    virtual bool send_raw(client_id id, const void* data, std::uint16_t size) override;
    virtual std::uint16_t recv_raw(client_id id, void* data, std::uint16_t size) override;

    bool send_message(client_id id, const std::string& message);

public:
        // persistent between update calls
    bool is_open() const;

    size_t get_client_count() const;
    client_id get_client(size_t index) const;

    size_t get_new_client_count() const;
    client_id get_new_client(size_t index) const;

    size_t get_lost_client_count() const;
    client_id get_lost_client(size_t index) const;

    size_t get_message_count() const;
    const std::pair<client_id, std::string>& get_message(size_t index) const;

public:
    void disconnect_user(client_id);

public:
    inline ~server_tcp() { close(true); }

private:
    app_protocol* m_app_protocol = nullptr;

    struct swarm;
    swarm* m_swarm = nullptr;
};

//------------------------------------------------------------------------------
}
