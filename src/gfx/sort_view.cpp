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

sort_view::sort_view([[maybe_unused]] sort_view_config const& cfg, std::vector<core::element_t> const& data)
{
    constexpr int num_vertices_per_rect = 4;
    m_data.reserve(data.size() * num_vertices_per_rect);

    constexpr int num_indices_per_rect = 6;
    m_index_data.reserve(data.size() * num_indices_per_rect);

    auto divide = [](core::element_t const a, core::element_t const b) -> float {
        return (static_cast<float>(a) / static_cast<float>(b)) * 2 - 1.0F;
    };

    unsigned int vertex_index = 0;

    for(core::element_t i = 0; i < data.size(); ++i) {
        // Order of vertices:
        // 0 - top left
        // 1 - top right
        // 2 - bottom right
        // 3 - bottom left
        std::array<vertex, 4> v;

        v[0].x = v[3].x = divide(i, data.size());
        v[1].x = v[2].x = divide(i + 1, data.size());

        v[0].y = v[1].y = divide(data[i], data.size());
        v[2].y = v[3].y = -1.0F;

        v[0].col = v[1].col = v[2].col = v[3].col = { 1.0F, 0.0F, 0.0F, 1.0F };

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

} // namespace gfx
