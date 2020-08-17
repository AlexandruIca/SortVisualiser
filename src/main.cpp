#include "algorithm/algorithm.hpp"
#include "algorithm/random.hpp"
#include "event/event.hpp"
#include "gfx/graphics.hpp"
#include "gfx/sort_view.hpp"
#include "gfx/window.hpp"
#include "log/log.hpp"

#include <chrono>
#include <numeric>
#include <stdexcept>
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
    try {
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
        cfg.type = gfx::view_type::point;
        cfg.color_type = gfx::color_gradient{ { 0.5F, 0.0F, 0.0F, 1.0F }, { 1.0F, 0.0F, 0.0F, 1.0F } }; // NOLINT
        cfg.highlight_color = gfx::color{ 0.0F, 0.6F, 0.0F, 1.0F };                                     // NOLINT

        gfx::sort_view view{ cfg, data };

        core::array input{ data };
        std::thread sort_thread{ [&input] { core::algorithm::radix_sort(input); } };
        auto& ev = core::event_manager::instance();

        using namespace std::chrono;
        using namespace std::chrono_literals;

        constexpr auto delay = 15ms;
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
                    view.compare(event.i, event.j);
                    break;
                }
                case core::event_type::modify: {
                    TRACE("[Consumer] Modified #{} with {}", event.i, event.j);
                    view.modify(event.i, event.j);
                    break;
                }
                case core::event_type::swap: {
                    TRACE("[Consumer] Swapped #{} with #{}", event.i, event.j);
                    view.swap(event.i, event.j);
                    break;
                }
                case core::event_type::end: {
                    TRACE("[Consumer] Ended sorting");
                    view.end();
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
    catch(std::exception const& e) {
        TRACE("Exception thrown in main: {}", e.what());
    }
}
