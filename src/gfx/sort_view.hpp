#ifndef SORTVIS_SORT_VIEW_HPP
#define SORTVIS_SORT_VIEW_HPP
#pragma once

#include <variant>
#include <vector>

#include "event/event.hpp"
#include "graphics.hpp"

namespace gfx {

enum class view_type
{
    rect,
    point
};

struct color_gradient
{
    color from;
    color to;
};

struct sort_view_config
{
    view_type type;
    std::variant<color, color_gradient> color_type;
};

class sort_view
{
private:
    struct vertex
    {
        float x{ 0.0F };
        float y{ 0.0F };
        color col;
    };

    std::vector<vertex> m_data;
    std::vector<unsigned int> m_index_data;
    unsigned int m_vao_id = 0;
    unsigned int m_vbo_id = 0;
    unsigned int m_shader_id = 0;

    inline static char const s_vertex_shader_source[] = R"(#version 330 core

    layout(location = 0) in vec2 position;
    layout(location = 1) in vec4 color;

    out vec4 vertex_color;

    void main()
    {
        gl_Position = vec4(position.xy, 0.0, 1.0);
        vertex_color = color;
    }
    )";

    inline static char const s_fragment_shader_source[] = R"(#version 330 core

    in vec4 vertex_color;
    out vec4 output_color;

    void main()
    {
        output_color = vertex_color;
    }
    )";

    enum class shader_type
    {
        vertex,
        fragment
    };

    [[nodiscard]] static auto create_shader(shader_type type) noexcept -> unsigned int;
    [[nodiscard]] static auto create_program(unsigned int vs, unsigned int fs) noexcept -> unsigned int;

public:
    sort_view() = delete;
    sort_view(sort_view const&) = default;
    sort_view(sort_view&&) noexcept = default;
    ~sort_view() noexcept;

    explicit sort_view(sort_view_config const& cfg, std::vector<core::element_t> const& data);

    auto operator=(sort_view const&) -> sort_view& = default;
    auto operator=(sort_view&&) noexcept -> sort_view& = default;

    auto draw() const noexcept -> void;
};

} // namespace gfx

#endif // !SORTVIS_SORT_VIEW_HPP
