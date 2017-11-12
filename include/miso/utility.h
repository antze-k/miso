// (C) unresolved-external@singu-lair.com released under the MIT license (see LICENSE)

#pragma once

#include <cstddef>
#include <string>

namespace miso {
//------------------------------------------------------------------------------

class noncopyable
{
protected:
    noncopyable() {}
    noncopyable(noncopyable&&) = default;
    noncopyable& operator=(noncopyable&&) = default;

private:
    noncopyable(const noncopyable&) = delete;
    noncopyable& operator=(const noncopyable&) = delete;
};

//------------------------------------------------------------------------------

class nonmovable
{
protected:
    nonmovable() {}
    nonmovable(const nonmovable&) = default;
    nonmovable& operator=(const nonmovable&) = default;

private:
    nonmovable(nonmovable&&) = delete;
    nonmovable& operator=(nonmovable&&) = delete;
};

//------------------------------------------------------------------------------
}
