#include "sort_view.hpp"
#include "log/log.hpp"

#include <glad/glad.h>

#include <array>
#include <cstddef>

#include <fmt/format.h>

namespace gfx {

auto sort_view::create_shader(sort_view::shader_type const type) noexcept -> unsigned int
{
    auto const* source =
        (type == sort_view::shader_type::vertex) ? s_vertex_shader_source : s_fragment_shader_source; // NOLINT

    unsigned int shader =
        glCreateShader((type == sort_view::shader_type::vertex) ? GL_VERTEX_SHADER : GL_FRAGMENT_SHADER);
    glShaderSource(shader, 1, &source, nullptr);
    glCompileShader(shader);

    int success = 0;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);

    if(success == 0) {
        int length = 0;
        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &length);

        std::string msg;
        msg.resize(static_cast<std::size_t>(length));

        glGetShaderInfoLog(shader, length, nullptr, msg.data());
        ERROR("Could not compile OpenGL {} shader: {}",
              (type == sort_view::shader_type::vertex) ? "vertex" : "fragment",
              msg);
    }

    return shader;
}

auto sort_view::create_program(unsigned int const vs, unsigned int const fs) noexcept -> unsigned int
{
    unsigned int const program = glCreateProgram();
    glAttachShader(program, vs);
    glAttachShader(program, fs);
    glLinkProgram(program);

    int success = 0;
    glGetProgramiv(program, GL_LINK_STATUS, &success);

    if(success == 0) {
        std::string msg;
        int length = 0;

        glGetProgramiv(program, GL_INFO_LOG_LENGTH, &length);
        msg.resize(static_cast<std::size_t>(length));

        glGetProgramInfoLog(program, length, nullptr, msg.data());
        ERROR("Couldn't create shader program: {}", msg);
    }

    glDeleteShader(vs);
    glDeleteShader(fs);

    return program;
}

sort_view::sort_view(sort_view_config const& cfg, std::vector<core::element_t> const& data)
{
    m_data_copy = data;

    constexpr int num_vertices_per_rect = s_num_vertices_per_rect;
    m_data.reserve(data.size() * num_vertices_per_rect);

    constexpr int num_indices_per_rect = 6;
    m_index_data.reserve(data.size() * num_indices_per_rect);

    auto lerp = [](float const a, float const b, float const t) -> float { return a * (1.0F - t) + t * b; };

    auto const& color_type = cfg.color_type;
    if(auto const* c = std::get_if<color>(&color_type)) {
        m_rect_color = *c;
    }
    m_highlight_color = cfg.highlight_color;

    if(cfg.type == view_type::rect) {
        m_generate_vertices = [data_size = data.size()](std::array<vertex, s_num_vertices_per_rect>& v,
                                                        core::element_t const i,
                                                        core::element_t const val) -> void {
            v[0].x = v[3].x = divide(i, data_size);
            v[1].x = v[2].x = divide(i + 1, data_size);

            v[0].y = v[1].y = divide(val, data_size);
            v[2].y = v[3].y = -1.0F;
        };
    }
    else {
        m_generate_vertices = [data_size = data.size()](std::array<vertex, s_num_vertices_per_rect>& v,
                                                        core::element_t const i,
                                                        core::element_t const val) -> void {
            v[0].x = v[3].x = divide(i, data_size);
            v[1].x = v[2].x = divide(i + 1, data_size);

            v[0].y = v[1].y = divide(val, data_size);
            v[2].y = v[3].y = divide(val - 1, data_size);
        };
    }

    if(auto const* g = std::get_if<color_gradient>(&color_type)) {
        m_generate_color = [g, &lerp, data_size = data.size()](core::element_t const val) -> color {
            auto c = color{ 0.0F, 0.0F, 0.0F, 1.0F };
            auto const t = divide(val, data_size);
            c.r = lerp(g->from.r, g->to.r, t);
            c.g = lerp(g->from.g, g->to.g, t);
            c.b = lerp(g->from.b, g->to.b, t);
            return c;
        };
    }
    else {
        m_generate_color = [this](core::element_t const) -> color { return m_rect_color; };
    }

    unsigned int vertex_index = 0;

    for(core::element_t i = 0; i < data.size(); ++i) {
        // Order of vertices:
        // 0 - top left
        // 1 - top right
        // 2 - bottom right
        // 3 - bottom left
        std::array<vertex, 4> v;
        m_generate_vertices(v, i, data[i]);
        v[0].col = v[1].col = v[2].col = v[3].col = m_generate_color(data[i]);

        m_data.insert(m_data.end(), v.begin(), v.end());

        m_index_data.push_back(vertex_index);
        m_index_data.push_back(vertex_index + 3);
        m_index_data.push_back(vertex_index + 2);
        m_index_data.push_back(vertex_index + 2);
        m_index_data.push_back(vertex_index);
        m_index_data.push_back(vertex_index + 1);

        vertex_index += 4;
    }

    glGenVertexArrays(1, &m_vao_id);
    glBindVertexArray(m_vao_id);

    glGenBuffers(1, &m_vbo_id);
    glBindBuffer(GL_ARRAY_BUFFER, m_vbo_id);
    glBufferData(GL_ARRAY_BUFFER, static_cast<int>(m_data.size() * sizeof(vertex)), m_data.data(), GL_DYNAMIC_DRAW);

    glVertexAttribPointer(
        0, 2, GL_FLOAT, GL_FALSE, sizeof(vertex), reinterpret_cast<void*>(offsetof(vertex, x))); // NOLINT
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(
        1, 4, GL_FLOAT, GL_FALSE, sizeof(vertex), reinterpret_cast<void*>(offsetof(vertex, col))); // NOLINT
    glEnableVertexAttribArray(1);

    unsigned int ibo = 0;
    glGenBuffers(1, &ibo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER,
                 static_cast<int>(m_index_data.size() * sizeof(unsigned int)),
                 m_index_data.data(),
                 GL_STATIC_DRAW);

    auto const vs = create_shader(shader_type::vertex);
    auto const fs = create_shader(shader_type::fragment);
    m_shader_id = sort_view::create_program(vs, fs);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

sort_view::~sort_view() noexcept
{
    glDeleteBuffers(1, &m_vbo_id);
    glDeleteVertexArrays(1, &m_vao_id);
    glDeleteProgram(m_shader_id);
}

auto sort_view::draw() const noexcept -> void
{
    glUseProgram(m_shader_id);
    glBindVertexArray(m_vao_id);

    glDrawElements(GL_TRIANGLES, static_cast<int>(m_index_data.size()), GL_UNSIGNED_INT, nullptr);

    glBindVertexArray(0);
    glUseProgram(0);
}

auto sort_view::undo_previous_event() -> void
{
    for(auto const& c : m_last_color) {
        this->update_rect_color(c.first, c.second);
    }

    m_last_color.clear();
}

auto sort_view::update_rect_color(core::element_t const index, color const& col) -> void
{
    constexpr core::element_t num_vertices_per_rect = s_num_vertices_per_rect;
    core::element_t const data_offset = index * num_vertices_per_rect;

    for(core::element_t offset = 0; offset < num_vertices_per_rect; ++offset) {
        m_data[data_offset + offset].col = col;
    }

    glBindBuffer(GL_ARRAY_BUFFER, m_vbo_id);
    glBufferSubData(GL_ARRAY_BUFFER,
                    static_cast<GLintptr>(data_offset * sizeof(vertex)),
                    num_vertices_per_rect * sizeof(vertex),
                    &m_data[data_offset]);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

auto sort_view::access(core::element_t const i) -> void
{
    this->undo_previous_event();
    // m_last_color.emplace_back(i, m_data[i * s_num_vertices_per_rect].col);
    m_last_color.emplace_back(i, m_generate_color(m_data_copy[i]));
    this->update_rect_color(i, m_highlight_color);
}

auto sort_view::swap(core::element_t const i, core::element_t const j) -> void
{
    this->undo_previous_event();

    std::swap(m_data_copy[i], m_data_copy[j]);

    constexpr core::element_t num_vertices_per_rect = s_num_vertices_per_rect;

    for(core::element_t offset = 0; offset < num_vertices_per_rect; ++offset) {
        std::swap(m_data[i * num_vertices_per_rect + offset].y, m_data[j * num_vertices_per_rect + offset].y);
        std::swap(m_data[i * num_vertices_per_rect + offset].col, m_data[j * num_vertices_per_rect + offset].col);
    }

    for(auto const index : { i, j }) {
        // m_last_color.emplace_back(index, m_data[index * num_vertices_per_rect].col);
        m_last_color.emplace_back(index, m_generate_color(m_data_copy[index]));
        this->update_rect_color(index, m_highlight_color);
    }
}

auto sort_view::compare(core::element_t const i, core::element_t const j) -> void
{
    this->undo_previous_event();

    for(auto const index : { i, j }) {
        // m_last_color.emplace_back(index, m_data[index * s_num_vertices_per_rect].col);
        m_last_color.emplace_back(index, m_generate_color(m_data_copy[index]));
        this->update_rect_color(index, m_highlight_color);
    }
}

auto sort_view::divide(core::element_t const a, core::element_t const b) -> float
{
    return (static_cast<float>(a) / static_cast<float>(b)) * 2 - 1.0F;
}

auto sort_view::modify(core::element_t const i, core::element_t const val) -> void
{
    this->undo_previous_event();

    m_data_copy[i] = val;
    std::array<vertex, s_num_vertices_per_rect> v;
    m_generate_vertices(v, i, val);

    for(core::element_t offset = 0; offset < v.size(); ++offset) {
        m_data[i * s_num_vertices_per_rect + offset] = v.at(offset);
    }

    v[0].col = v[1].col = v[2].col = v[3].col = m_generate_color(val);
    this->update_rect_color(i, v[0].col);
}

auto sort_view::end() -> void
{
    this->undo_previous_event();
}

} // namespace gfx
