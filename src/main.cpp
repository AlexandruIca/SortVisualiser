#include "algorithm/algorithm.hpp"
#include "algorithm/random.hpp"
#include "event/event.hpp"
#include "gfx/graphics.hpp"
#include "gfx/sort_view.hpp"
#include "gfx/window.hpp"
#include "log/log.hpp"

#include <chrono>
#include <numeric>
#include <thread>

[[nodiscard]] auto generate_data(core::element_t const count) -> std::vector<core::element_t>
{
    std::vector<core::element_t> result{};

    result.resize(count);
    std::iota(result.begin(), result.end(), 1);
    core::random_shuffle(result);

    return result;
}

auto main(int, char*[]) noexcept -> int
{
    gfx::window wnd{ "SortVisualizer" };

    bool process_next_event = false;
    bool pause_after_iteration = false;

    wnd.on_key_press([&process_next_event, &pause_after_iteration](gfx::key_event const ev) {
        if(ev == gfx::key_event::right) {
            TRACE("RIGHT arrow pressed");
            process_next_event = true;
            pause_after_iteration = true;
        }
        else if(ev == gfx::key_event::space) {
            TRACE("SPACE key pressed");
            process_next_event = !process_next_event;
        }
    });

    gfx::set_clear_color(gfx::color{});

    auto const data = generate_data(10);
    gfx::sort_view_config cfg{};
    gfx::sort_view view{ cfg, data };

    core::array input{ data };
    std::thread sort_thread{ [&input] {
        static_cast<void>(input[0]);
        static_cast<void>(input[1]);
        core::algorithm::bubble_sort(input);
    } };
    auto& ev = core::event_manager::instance();

    using namespace std::chrono;
    using namespace std::chrono_literals;

    constexpr auto delay = 150ms;
    auto start = steady_clock::now();

    while(!wnd.should_close()) {
        wnd.handle_events();

        auto end = steady_clock::now();
        auto const duration = end - start;

        if(process_next_event && duration >= delay && !ev.empty()) {
            start = end;
            auto const event = ev.pop();

            switch(event.type) {
            case core::event_type::access: {
                TRACE("[Consumer] Accessed #{}", event.i);
                view.access(event.i);
                break;
            }
            case core::event_type::compare: {
                TRACE("[Consumer] Compared #{} with #{}", event.i, event.j);
                break;
            }
            case core::event_type::modify: {
                TRACE("[Consumer] Modified #{} with #{}", event.i, event.j);
                break;
            }
            case core::event_type::swap: {
                TRACE("[Consumer] Swapped #{} with #{}", event.i, event.j);
                break;
            }
            default: {
                break;
            }
            }
        }

        if(pause_after_iteration) {
            process_next_event = false;
            pause_after_iteration = false;
        }

        gfx::clear();
        view.draw();

        wnd.swap_buffers();
    }

    sort_thread.join();
}
