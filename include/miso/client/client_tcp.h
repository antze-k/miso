// (C) unresolved-external@singu-lair.com released under the MIT license (see LICENSE)

#pragma once

#include "miso/utility.h"
#include "miso/client/client_interface.h"

#include <utility>

namespace miso {
//------------------------------------------------------------------------------

class generic_socket;
class app_protocol;

//------------------------------------------------------------------------------

class client_tcp final : public noncopyable, public nonmovable, public client_interface
{
public:
    struct configuration
    {
        static const size_t default_app_buffer_size = 8192;

        size_t app_input_buffer_size    = default_app_buffer_size;
        size_t app_output_buffer_size   = default_app_buffer_size;

        configuration() {}
    };

public:
    void set_app_protocol(app_protocol*);

    bool connect_async(const ip_address&, std::uint16_t port, const configuration& cfg = configuration());
    bool connect_wait(const ip_address&, std::uint16_t port, int wait_timeout_ms = -1, const configuration& cfg = configuration());
    bool assign(generic_socket&&, const configuration& cfg = configuration());
    void disconnect();

public:
    void update();

public:
    virtual bool send_raw(const void* data, std::uint16_t size) override;
    virtual std::uint16_t recv_raw(void* data, std::uint16_t size) override;

    bool send_message(const std::string& message);

public:
        // persistent between update calls
    bool is_open() const;
    bool is_connected() const;

    size_t get_message_count() const;
    const std::string& get_message(size_t index) const;

public:
    inline ~client_tcp() { disconnect(); }

private:
    app_protocol* m_app_protocol = nullptr;

    struct client_data;
    client_data* m_data = nullptr;
};

//------------------------------------------------------------------------------
}
