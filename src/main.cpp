#include "gfx/graphics.hpp"
#include "gfx/window.hpp"
#include "log/log.hpp"

auto main(int, char*[]) noexcept -> int
{
    gfx::window wnd{ "SortVisualizer" };

    wnd.on_key_press([](gfx::key_event const ev) {
        if(ev == gfx::key_event::right) {
            TRACE("RIGHT arrow pressed");
        }
        else if(ev == gfx::key_event::space) {
            TRACE("SPACE key pressed");
        }
    });

    gfx::set_clear_color(gfx::color{});

    while(!wnd.should_close()) {
        wnd.handle_events();

        gfx::clear();

        wnd.swap_buffers();
    }
}
