// (C) unresolved-external@singu-lair.com released under the MIT license (see LICENSE)

#include "miso/client/client_tcp.h"
#include "miso/socket/generic_socket.h"
#include "node_tcp.h"

#include <algorithm>
#include <vector>

namespace miso {
//------------------------------------------------------------------------------

struct client_tcp::client_data
{
    node_tcp node;

    std::vector<in_message> in_messages;

    inline client_data(generic_socket&& s, size_t app_inbuf_size, size_t app_outbuf_size)
        : node(0, std::move(s), app_inbuf_size, app_outbuf_size) {}
};

//------------------------------------------------------------------------------

void client_tcp::set_app_protocol(app_protocol* app_protocol)
{
    m_app_protocol = app_protocol;
}

//------------------------------------------------------------------------------

bool client_tcp::connect_async(const ip_address& address, std::uint16_t port, const configuration& cfg)
{
    generic_socket s;
    if (!s.configure((internet_protocol_t)address.get_type(), tcp, nonblocking, true))
        return false;
    if (!s.connect(address, port) && s.get_status().error != error_none)
        return false;

    return assign(std::move(s));
}

//------------------------------------------------------------------------------

bool client_tcp::connect_wait(const ip_address& address, std::uint16_t port, int wait_timeout_ms, const configuration& cfg)
{
    if (!connect_async(address, port, cfg))
        return false;

    return m_data->node.socket.proceed(wait_timeout_ms);
}

//------------------------------------------------------------------------------

bool client_tcp::assign(generic_socket&& s, const configuration& cfg)
{
    if (!s.is_open())
        return false;

    disconnect();

    try
    {
        const size_t app_inbuf_size = cfg.app_input_buffer_size > 0 ? cfg.app_input_buffer_size : configuration::default_app_buffer_size;
        const size_t app_outbuf_size = cfg.app_output_buffer_size > 0 ? cfg.app_output_buffer_size : configuration::default_app_buffer_size;
        m_data = new client_data(std::move(s), app_inbuf_size, app_outbuf_size);
    }
    catch (std::bad_alloc&)
    {
        disconnect();
        return false;
    }

    if (m_data == nullptr)
    {
        disconnect();
        return false;
    }

    return true;
}

//------------------------------------------------------------------------------

void client_tcp::disconnect()
{
    delete m_data;
    m_data = nullptr;
}

//------------------------------------------------------------------------------

void client_tcp::update()
{
    if (!m_data)
        return;

    m_data->in_messages.clear();

    if (!m_data->node.socket.proceed() && m_data->node.socket.get_status().error != error_none)
        m_data->node.socket.close();
    if (!m_data->node.stream_in() || !m_data->node.parse_in(m_app_protocol, m_data->in_messages) || !m_data->node.stream_out())
        m_data->node.socket.close();
    if (!m_data->node.socket.is_open())
        disconnect();
}

//------------------------------------------------------------------------------

bool client_tcp::send_raw(const void* data, std::uint16_t size)
{
    if (!m_data)
        return false;

    return m_data->node.stream_out(data, size);
}

//------------------------------------------------------------------------------

std::uint16_t client_tcp::recv_raw(void* data, std::uint16_t size)
{
    if (!m_data || size == 0)
        return 0;

    return (std::uint16_t)m_data->node.in.grab(data, size);
}

//------------------------------------------------------------------------------

bool client_tcp::send_message(const std::string& message)
{
    if (!m_data || !m_app_protocol)
        return false;

    size_t used;
    if (!m_app_protocol->push_out_message(message, &m_data->node.out.buf[m_data->node.out.top], m_data->node.out.left(), used))
        return false;

    m_data->node.out.top += used;

    return m_data->node.stream_out();
}

//------------------------------------------------------------------------------

size_t client_tcp::get_message_count() const
{
    return m_data ? m_data->in_messages.size() : 0;
}

//------------------------------------------------------------------------------

const std::string& client_tcp::get_message(size_t index) const
{
    static const std::string invalid_message;
    return (!m_data && index >= m_data->in_messages.size()) ? invalid_message : m_data->in_messages[index].second;
}

//------------------------------------------------------------------------------

bool client_tcp::is_open() const
{
    return m_data != nullptr;
}

//------------------------------------------------------------------------------

bool client_tcp::is_connected() const
{
    return m_data != nullptr && m_data->node.socket.get_status().up;
}

//------------------------------------------------------------------------------
}
