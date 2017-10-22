// (C) unresolved-external@singu-lair.com released under the MIT license (see LICENSE)

#include "miso/server/server_tcp.h"
#include "miso/socket/generic_socket.h"
#include "node_tcp.h"

#include <algorithm>
#include <vector>

namespace miso {
//------------------------------------------------------------------------------

struct server_tcp::swarm
{
    bool shut_down = false;

    generic_socket core;
    const size_t app_inbuf_size;
    const size_t app_outbuf_size;

    std::vector<node_tcp> clients;
    client_id last_client_id = 0;

    std::vector<client_id> new_clients;
    std::vector<client_id> lost_clients;

    std::vector<in_message> in_messages;

    inline swarm(generic_socket&& s, size_t app_inbuf_size, size_t app_outbuf_size)
        : core(std::move(s)), app_inbuf_size(app_inbuf_size), app_outbuf_size(app_outbuf_size) {}

    inline client_id next_client_id()
    {
        client_id next = ++last_client_id;
        if (next == invalid_client() || next == 0)
            next = last_client_id = 1;
        return next;
    }
};

//------------------------------------------------------------------------------

void server_tcp::set_app_protocol(app_protocol* app_protocol)
{
    m_app_protocol = app_protocol;
}

//------------------------------------------------------------------------------

bool server_tcp::open_ipv4(std::uint16_t port, const configuration& cfg)
{
    generic_socket s;
    if (!s.configure(ipv4, tcp, nonblocking, true))
        return false;
    if (!s.listen(port) && s.get_status().error != error_none)
        return false;

    return assign(std::move(s), cfg);
}

//------------------------------------------------------------------------------

bool server_tcp::open_ipv6(std::uint16_t port, const configuration& cfg)
{
    generic_socket s;
    if (!s.configure(ipv6, tcp, nonblocking, true))
        return false;
    if (!s.listen(port) && s.get_status().error != error_none)
        return false;

    return assign(std::move(s), cfg);
}

//------------------------------------------------------------------------------

bool server_tcp::assign(generic_socket&& s, const configuration& cfg)
{
    if (!s.is_configured())
        return false;

    close(true);

    try
    {
        const size_t app_inbuf_size = cfg.app_input_buffer_size > 0 ? cfg.app_input_buffer_size : configuration::default_app_buffer_size;
        const size_t app_outbuf_size = cfg.app_output_buffer_size > 0 ? cfg.app_output_buffer_size : configuration::default_app_buffer_size;
        m_swarm = new swarm(std::move(s), app_inbuf_size, app_outbuf_size);
    }
    catch (std::bad_alloc&)
    {
        close(true);
        return false;
    }

    if (m_swarm == nullptr)
    {
        close(true);
        return false;
    }

    return true;
}

//------------------------------------------------------------------------------

void server_tcp::close(bool free_data)
{
    if (free_data)
    {
        delete m_swarm;
        m_swarm = nullptr;
    }
    else if (m_swarm)
        m_swarm->shut_down = true;
}

//------------------------------------------------------------------------------

void server_tcp::update()
{
    if (!m_swarm)
        return;

    m_swarm->new_clients.clear();
    m_swarm->lost_clients.clear();
    m_swarm->in_messages.clear();

        // accept new clients

    generic_socket accepted;
    while (m_swarm->core.accept(accepted))
    {
        client_id next = m_swarm->next_client_id();
        m_swarm->clients.push_back(node_tcp(next, std::move(accepted), m_swarm->app_inbuf_size, m_swarm->app_outbuf_size));
        m_swarm->new_clients.push_back(next);
    }

        // i/o

    if (!m_swarm->core.proceed() && m_swarm->core.get_status().error != error_none)
        close(false);

    for (node_tcp& client : m_swarm->clients)
        if (!client.stream_in() || !client.parse_in(m_app_protocol, m_swarm->in_messages) || !client.stream_out())
            client.socket.close();

        // clean up lost clients

    size_t client_count = m_swarm->clients.size();
    for (size_t client_index = 0; client_index < client_count; /* ++client_index */)
    {
        if (!m_swarm->clients[client_index].socket.is_open() || !is_open())
        {
            m_swarm->lost_clients.push_back(m_swarm->clients[client_index].id);
            m_swarm->clients.erase(m_swarm->clients.begin() + client_index);
            --client_count;
        }
        else
            ++client_index;
    }
}

//------------------------------------------------------------------------------

bool server_tcp::send_raw(client_id id, const void* data, std::uint16_t size)
{
    if (!m_swarm)
        return false;

    auto found = std::find_if(m_swarm->clients.begin(), m_swarm->clients.end(), [&id](node_tcp& client) { return client.id == id; });
    if (found == m_swarm->clients.end())
        return false;

    return found->stream_out(data, size);
}

//------------------------------------------------------------------------------

std::uint16_t server_tcp::recv_raw(client_id id, void* data, std::uint16_t size)
{
    if (!m_swarm || size == 0)
        return 0;

    auto found = std::find_if(m_swarm->clients.begin(), m_swarm->clients.end(), [&id](node_tcp& client) { return client.id == id; });
    if (found == m_swarm->clients.end())
        return 0;

    return (std::uint16_t)found->in.grab(data, size);
}

//------------------------------------------------------------------------------

bool server_tcp::send_message(client_id id, const std::string& message)
{
    if (!m_swarm || !m_app_protocol)
        return false;

    auto found = std::find_if(m_swarm->clients.begin(), m_swarm->clients.end(), [&id](node_tcp& client) { return client.id == id; });
    if (found == m_swarm->clients.end())
        return false;

    size_t used;
    if (!m_app_protocol->push_out_message(message, &found->out.buf[found->out.top], found->out.left(), used))
        return false;

    found->out.top += used;

    return found->stream_out();
}

//------------------------------------------------------------------------------

size_t server_tcp::get_client_count() const
{
    return m_swarm ? m_swarm->clients.size() : 0;
}

//------------------------------------------------------------------------------

server_tcp::client_id server_tcp::get_client(size_t index) const
{
    return (!m_swarm && index >= m_swarm->clients.size()) ? invalid_client() : m_swarm->clients[index].id;
}

//------------------------------------------------------------------------------

size_t server_tcp::get_new_client_count() const
{
    return m_swarm ? m_swarm->new_clients.size() : 0;
}

//------------------------------------------------------------------------------

server_tcp::client_id server_tcp::get_new_client(size_t index) const
{
    return (!m_swarm && index >= m_swarm->new_clients.size()) ? invalid_client() : m_swarm->new_clients[index];
}

//------------------------------------------------------------------------------

size_t server_tcp::get_lost_client_count() const
{
    return m_swarm ? m_swarm->lost_clients.size() : 0;
}

//------------------------------------------------------------------------------

server_tcp::client_id server_tcp::get_lost_client(size_t index) const
{
    return (!m_swarm && index >= m_swarm->lost_clients.size()) ? invalid_client() : m_swarm->lost_clients[index];
}

//------------------------------------------------------------------------------

size_t server_tcp::get_message_count() const
{
    return m_swarm ? m_swarm->in_messages.size() : 0;
}

//------------------------------------------------------------------------------

const std::pair<server_tcp::client_id, std::string>& server_tcp::get_message(size_t index) const
{
    static const std::pair<client_id, std::string> invalid_message({ invalid_client(), std::string() });
    return (!m_swarm && index >= m_swarm->in_messages.size()) ? invalid_message : m_swarm->in_messages[index];
}

//------------------------------------------------------------------------------

void server_tcp::disconnect_user(client_id id)
{
    if (!m_swarm)
        return;

    auto found = std::find_if(m_swarm->clients.begin(), m_swarm->clients.end(), [&id](node_tcp& client) { return client.id == id; });
    if (found == m_swarm->clients.end())
        return;

    m_swarm->lost_clients.push_back(found->id);
    m_swarm->clients.erase(found);
}

//------------------------------------------------------------------------------

bool server_tcp::is_open() const
{
    return m_swarm != nullptr && !m_swarm->shut_down;
}

//------------------------------------------------------------------------------
}
