#ifndef SORTVIS_GFX_WINDOW_HPP
#define SORTVIS_GFX_WINDOW_HPP
#pragma once

#include <functional>
#include <string>

struct SDL_Window;
struct SDL_Renderer;

using SDL_GLContext = void*;

namespace gfx {

enum class key_event
{
    space,
    right,
    s
};

class window
{
private:
    SDL_Window* m_window{ nullptr };
    SDL_Renderer* m_renderer{};
    SDL_GLContext m_gl_context{ nullptr };

    bool m_should_close{ false };

    static constexpr int s_default_width = 1280;
    static constexpr int s_default_height = 720;

    int m_width{ s_default_width };
    int m_height{ s_default_width };

    std::function<void(key_event)> m_on_key_press = [](key_event) {};

public:
    window() = delete;
    window(window const&) noexcept = default;
    window(window&&) noexcept = default;
    ~window() noexcept;

    explicit window(std::string const& title, int w = s_default_width, int h = s_default_height) noexcept;

    auto operator=(window const&) noexcept -> window& = default;
    auto operator=(window&&) noexcept -> window& = default;

    [[nodiscard]] auto should_close() const noexcept -> bool;

    [[nodiscard]] auto width() const noexcept -> int;
    [[nodiscard]] auto height() const noexcept -> int;

    auto handle_events() noexcept -> void;
    auto swap_buffers() noexcept -> void;

    template<typename F>
    auto on_key_press(F func) -> void
    {
        m_on_key_press = func;
    }
};

} // namespace gfx

#endif // !SORTVIS_GFX_WINDOW_HPP
