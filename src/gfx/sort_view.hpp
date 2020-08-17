#ifndef SORTVIS_SORT_VIEW_HPP
#define SORTVIS_SORT_VIEW_HPP
#pragma once

#include <array>
#include <functional>
#include <optional>
#include <utility>
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
    color highlight_color;
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

    static constexpr color s_red = color{ 1.0F, 0.0F, 0.0F, 1.0F };
    color m_rect_color = s_red;
    static constexpr color s_green = color{ 0.0F, 1.0F, 0.0F, 1.0F };
    color m_highlight_color = s_green;

    static constexpr core::element_t s_num_vertices_per_rect = 4;
    std::function<void(std::array<vertex, s_num_vertices_per_rect>&, core::element_t, core::element_t)>
        m_generate_vertices = [](std::array<vertex, s_num_vertices_per_rect>&, core::element_t, core::element_t) {};
    std::function<color(core::element_t)> m_generate_color = [](core::element_t) -> color {
        return { 0.0F, 0.0F, 0.0F, 1.0F };
    };

    std::vector<std::pair<std::size_t, color>> m_last_color{};

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

    [[nodiscard]] static auto divide(core::element_t a, core::element_t b) -> float;

    [[nodiscard]] static auto create_shader(shader_type type) noexcept -> unsigned int;
    [[nodiscard]] static auto create_program(unsigned int vs, unsigned int fs) noexcept -> unsigned int;

    auto undo_previous_event() -> void;
    auto update_rect_color(core::element_t index, color const& col) -> void;

public:
    sort_view() = delete;
    sort_view(sort_view const&) = default;
    sort_view(sort_view&&) noexcept = default;
    ~sort_view() noexcept;

    explicit sort_view(sort_view_config const& cfg, std::vector<core::element_t> const& data);

    auto operator=(sort_view const&) -> sort_view& = default;
    auto operator=(sort_view&&) noexcept -> sort_view& = default;

    auto draw() const noexcept -> void;

    auto access(core::element_t i) -> void;
    auto swap(core::element_t i, core::element_t j) -> void;
    auto compare(core::element_t i, core::element_t j) -> void;
    auto modify(core::element_t i, core::element_t val) -> void;
    auto end() -> void;
};

} // namespace gfx

#endif // !SORTVIS_SORT_VIEW_HPP
