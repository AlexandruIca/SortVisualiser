#include "gfx/graphics.hpp"
#include "gfx/sort_view.hpp"
#include "gfx/window.hpp"
#include "log/log.hpp"

#include <numeric>
#include <random>

[[nodiscard]] auto generate_data(core::element_t const count) -> std::vector<core::element_t>
{
    std::mt19937 rng{ std::random_device{}() };
    std::vector<core::element_t> result{};

    result.resize(count);
    std::iota(result.begin(), result.end(), 1);
    std::shuffle(result.begin(), result.end(), rng);

    return result;
}

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

    auto const data = generate_data(10);
    gfx::sort_view_config cfg{};
    gfx::sort_view view{ cfg, data };

    while(!wnd.should_close()) {
        wnd.handle_events();

        gfx::clear();
        view.draw();

        wnd.swap_buffers();
    }
}
