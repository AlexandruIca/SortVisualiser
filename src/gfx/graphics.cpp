#include "graphics.hpp"

#include <glad/glad.h>

namespace gfx {

auto set_clear_color(color const& c) noexcept -> void
{
    glClearColor(c.r, c.g, c.b, c.a);
}

auto clear() noexcept -> void
{
    glClear(GL_COLOR_BUFFER_BIT);
}

} // namespace gfx
