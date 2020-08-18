#include "window.hpp"
#include "log/log.hpp"

#include <SDL.h>
#include <glad/glad.h>

namespace gfx {

window::window(std::string const& title, int const w, int const h) noexcept
    : m_width{ w }
    , m_height{ h }
{
    if(SDL_Init(SDL_INIT_VIDEO) != 0) {
        FATAL("Couldn't create an SDL window: {}", SDL_GetError());
    }

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

    m_window = SDL_CreateWindow(
        title.c_str(), SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, w, h, SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);
    ASSERT(m_window != nullptr);

    m_renderer = SDL_CreateRenderer(m_window, -1, SDL_RENDERER_ACCELERATED);
    ASSERT(m_renderer != nullptr);

    m_gl_context = SDL_GL_CreateContext(m_window);

    if(gladLoadGLLoader(static_cast<GLADloadproc>(SDL_GL_GetProcAddress)) == 0) {
        FATAL("Failed to initialize OpenGL context");
    }

    INFO("OpenGL context created! Version {}.{}", GLVersion.major, GLVersion.minor);
}

window::~window() noexcept
{
    SDL_GL_DeleteContext(m_gl_context);
    SDL_DestroyRenderer(m_renderer);
    SDL_DestroyWindow(m_window);
    SDL_Quit();
}

auto window::should_close() const noexcept -> bool
{
    return m_should_close;
}

auto window::width() const noexcept -> int
{
    return m_width;
}

auto window::height() const noexcept -> int
{
    return m_height;
}

auto window::handle_events() noexcept -> void
{
    SDL_Event e;

    while(static_cast<bool>(SDL_PollEvent(&e))) {
        switch(e.type) {
        case SDL_WINDOWEVENT: {
            if(e.window.event == SDL_WINDOWEVENT_SIZE_CHANGED) {
                m_width = e.window.data1;
                m_height = e.window.data2;
                glViewport(0, 0, m_width, m_height);
            }
            break;
        }
        case SDL_KEYDOWN: {
            switch(e.key.keysym.sym) {
            case SDLK_ESCAPE: {
                m_should_close = true;
                break;
            }
            case SDLK_SPACE: {
                m_on_key_press(key_event::space);
                break;
            }
            case SDLK_RIGHT: {
                m_on_key_press(key_event::right);
                break;
            }
            case SDLK_s: {
                m_on_key_press(key_event::s);
                break;
            }
            default: {
                break;
            }
            }
            break;
        }
        case SDL_QUIT: {
            m_should_close = true;
            break;
        }
        default: {
            break;
        }
        }
    }
}

auto window::swap_buffers() noexcept -> void
{
    SDL_GL_SwapWindow(m_window);
}

} // namespace gfx
