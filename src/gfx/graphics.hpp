#ifndef SORTVIS_GRAPHICS_HPP
#define SORTVIS_GRAPHICS_HPP
#pragma once

namespace gfx {

struct color
{
    float r{ 0.0F };
    float g{ 0.0F };
    float b{ 0.0F };
    float a{ 1.0F };
};

auto set_clear_color(color const& c) noexcept -> void;
auto clear() noexcept -> void;

} // namespace gfx

#endif // !SORTVIS_GRAPHICS_HPP
