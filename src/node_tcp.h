// (C) unresolved-external@singu-lair.com released under the MIT license (see LICENSE)

#pragma once

#include "miso/server/server_interface.h"  // client_id
#include "miso/socket/generic_socket.h"
#include "miso/protocol/app_protocol.h"

#include <algorithm>
#include <cstring>
#include <vector>

namespace miso {
//------------------------------------------------------------------------------

struct app_buffer
{
    std::vector<char> buf;
    size_t top = 0;

    inline app_buffer(size_t size) : buf(size) {}
    inline size_t left() const { return buf.size() - top; }

    inline void shift(size_t size)
    {
        if (size < top) { memmove(&buf[0], &buf[size], top - size); top -= size; }
        else top = 0;
    }

    inline size_t grab(void* data, size_t size)
    {
        if (top == 0) return 0;
        const size_t to_grab = std::min(size, top);
        if (data) memcpy(data, &buf[0], to_grab);
        shift(to_grab);
        return to_grab;
    }

    inline bool push(const void* data, size_t size)
    {
        if (left() < size)
            return false;

        memcpy(&buf[top], data, size);
        top += size;
        return true;
    }
};

//------------------------------------------------------------------------------

using in_message = std::pair<server_interface::client_id, std::string>;

//------------------------------------------------------------------------------

struct node_tcp
{
    server_interface::client_id id;
    generic_socket socket;
    app_buffer in;
    app_buffer out;

    node_tcp() = default;
    node_tcp(node_tcp&&) = default;
    node_tcp& operator=(node_tcp&&) = default;

    node_tcp(const node_tcp&) = delete;
    node_tcp& operator=(const node_tcp&) = delete;

    node_tcp(server_interface::client_id id, generic_socket&& socket, size_t app_inbuf_size, size_t app_outbuf_size)
        : id(id), socket(std::move(socket)), in(app_inbuf_size), out(app_outbuf_size) {}

    bool stream_in()
    {
        size_t left = in.left();
        if (left == 0)
            return false;  // input buffer overflow

        const int received = socket.is_open() ? (socket.get_status().up ? socket.recv(&in.buf[in.top], left) : 0) : -1;
        if (received < 0)
            return false;

        in.top += received;
        return true;
    }

    bool parse_in(app_protocol* protocol, std::vector<in_message>& in_messages)
    {
        if (!protocol || in.top == 0)
            return true;

        size_t used;
        std::string message;
        while (protocol->pop_in_message(&in.buf[0], in.top, used, message))
        {
            in_messages.emplace_back(id, message);
            if (used == 0) return false;
            in.shift(used);
        }

        return true;
    }

    bool stream_out()
    {
        if (!socket.is_open())
            return false;

        if (!socket.get_status().up)
            return true;

        if (out.top == 0)
            return true;

        int sent = socket.send(&out.buf[0], out.top);
        if (sent < 0)
            return false;

        out.shift(sent);
        return true;
    }

    bool stream_out(const void* data, size_t size)
    {
        if (!socket.is_open())
            return false;

        if (!socket.get_status().up)
            return out.push((const char*)data, size);

        if (out.top > 0)
        {
            int sent = socket.send(&out.buf[0], out.top);
            if (sent < 0)
                return false;

            out.shift(sent);
            if (out.top > 0)
                return false;
        }

        if (size == 0)
            return true;

        if (!data)
            return false;

        int sent = socket.send(data, size);
        if (sent < 0)
            return false;

        if ((size_t)sent == size)
            return true;

        return out.push((const char*)data + sent, size - sent);
    }
};

//------------------------------------------------------------------------------
}
