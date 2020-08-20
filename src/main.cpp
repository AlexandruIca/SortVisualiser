#include "algorithm/algorithm.hpp"
#include "algorithm/random.hpp"
#include "audio/audio.hpp"
#include "event/event.hpp"
#include "gfx/graphics.hpp"
#include "gfx/sort_view.hpp"
#include "gfx/window.hpp"
#include "log/log.hpp"

#include <docopt/docopt.h>

#include <chrono>
#include <iostream>
#include <map>
#include <numeric>
#include <stdexcept>
#include <string>
#include <thread>
#include <unordered_map>

[[nodiscard]] auto generate_data(core::element_t const count) -> std::vector<core::element_t>
{
    std::vector<core::element_t> result{};

    result.resize(count);
    std::iota(result.begin(), result.end(), 1);
    core::random_shuffle(result);

    return result;
}

char const g_usage[] = R"(SortVisualizer

Usage:
    SortVisualizer [-h | --help]
                      [--size=<num_rects>]
                      [--algorithm=<algo>]
                      [--type=<view_type>]
                      [--color=<rect_color>]
                      [(--color-from=<rect_color_from> --color-to=<rect_color_to>)]
                      [--highlight-color=<rect_hl_color>]
                      [--delay-ms=<delay>]

Options:
    -h --help                          Show this screen.
    --size=<num_rects>                 How many elements to sort.
    --algorithm=<algo>                 What sorting algorithm to show [default: bubble_sort].
    --type=<view_type>                 View rects or 'points'(values: 'rect' | 'point').
    --color=<rect_color>               The color of the rects.
    --color-from=<rect_color_from>     Gradient color begin.
    --color-to=<rect_color_to>         Gradient color end.
    --highlight-color=<rect_hl_color>  Color to highlight elements.
    --delay-ms=<dely>                  Delay time between sorting events in milliseconds [default: 15].
)";

using algorithm_t = void (*)(core::array&);

std::unordered_map<std::string, algorithm_t> g_algorithms = { { "count_sort", &core::algorithm::count_sort },
                                                              { "bubble_sort", &core::algorithm::bubble_sort },
                                                              { "insertion_sort", &core::algorithm::insertion_sort },
                                                              { "radix_sort", &core::algorithm::radix_sort },
                                                              { "radix_sort_simple",
                                                                &core::algorithm::radix_sort_simple },
                                                              { "quicksort", &core::algorithm::quicksort },
                                                              { "merge_sort", &core::algorithm::merge_sort } };

std::unordered_map<std::string, gfx::color> const g_colors = { { "red", { 1.0F, 0.0F, 0.0F, 1.0F } },
                                                               { "green", { 0.0F, 1.0F, 0.0F, 1.0F } },
                                                               { "blue", { 0.0F, 0.0F, 1.0F, 1.0F } },
                                                               { "white", { 1.0F, 1.0F, 1.0F, 1.0F } } };

auto configure(std::map<std::string, docopt::value> args,
               core::element_t& size,
               algorithm_t& algo,
               std::chrono::milliseconds& delay,
               gfx::sort_view_config& cfg) -> void
{
    if(args["--size"].isString()) {
        auto new_size = std::stoul(args["--size"].asString());

        constexpr int min_limit = 5;
        if(new_size < min_limit) {
            new_size = min_limit;
        }

        size = static_cast<core::element_t>(new_size);
    }
    if(args["--algorithm"].isString()) {
        auto const algorithm = args["--algorithm"].asString();

        if(g_algorithms.find(algorithm) != g_algorithms.end()) {
            algo = g_algorithms.at(algorithm);
        }
    }
    if(args["--type"].isString()) {
        auto const value = args["--type"].asString();

        if(value == "rect") {
            cfg.type = gfx::view_type::rect;
        }
        else if(value == "point") {
            cfg.type = gfx::view_type::point;
        }
    }
    if(args["--color"].isString()) {
        auto const str = args["--color"].asString();
        auto it = g_colors.find(str);

        if(it != g_colors.end()) {
            cfg.color_type = g_colors.at(str);
        }
    }
    else if(args["--color-from"].isString()) {
        auto const from_str = args["--color-from"].asString();
        auto const to_str = args["--color-to"].asString();

        auto it = g_colors.find(from_str);
        auto it2 = g_colors.find(to_str);

        if(it != g_colors.end() && it2 != g_colors.end()) {
            auto const from = g_colors.at(from_str);
            auto const to = g_colors.at(to_str);
            cfg.color_type = gfx::color_gradient{ from, to };
        }
    }
    if(args["--highlight-color"].isString()) {
        auto const color_str = args["--highlight-color"].asString();

        if(g_colors.find(color_str) != g_colors.end()) {
            cfg.highlight_color = g_colors.at(color_str);
        }
    }
    if(args["--delay-ms"].isString()) {
        auto const delay_ms = args["--delay-ms"].asString();
        delay = std::chrono::milliseconds(std::stoi(delay_ms));
    }
}

auto main(int argc, char* argv[]) noexcept -> int
{
    try {
        auto args =
            docopt::docopt(g_usage, { argv + 1, argv + argc }, /* show help: */ true, "SortVisualizer"); // NOLINT

        gfx::window wnd{ "SortVisualizer" };
        auto& sound = audio::audio_manager::instance();

        bool process_next_event = false;
        bool pause_after_iteration = false;

        wnd.on_key_press([&process_next_event, &pause_after_iteration, &sound](gfx::key_event const ev) {
            if(ev == gfx::key_event::right) {
                TRACE("RIGHT arrow pressed");
                process_next_event = true;
                pause_after_iteration = true;
            }
            else if(ev == gfx::key_event::space) {
                TRACE("SPACE key pressed");
                process_next_event = !process_next_event;
            }
            else if(ev == gfx::key_event::s) {
                TRACE("'S' key pressed");
                sound.sound_on() ? sound.turn_sound_off() : sound.turn_sound_on();
            }
        });

        gfx::set_clear_color(gfx::color{});

        using namespace std::chrono;
        using namespace std::chrono_literals;

        std::chrono::milliseconds delay = 15ms;

        core::element_t data_size = 10; // NOLINT
        constexpr gfx::color red{ 1.0F, 0.0F, 0.0F, 1.0F };
        constexpr gfx::color dark_red{ 0.5F, 0.0F, 0.0F, 1.0F };
        constexpr gfx::color green{ 0.0F, 0.8F, 0.0F, 1.0F };

        gfx::sort_view_config cfg{};
        cfg.type = gfx::view_type::rect;
        cfg.color_type = gfx::color_gradient{ dark_red, red };
        cfg.highlight_color = green;

        algorithm_t algo = &core::algorithm::bubble_sort;

        configure(args, data_size, algo, delay, cfg);

        constexpr double sound_delay = 20.0;
        sound.set_max(data_size);
        sound.set_delay(sound_delay);

        auto const data = generate_data(data_size);

        gfx::sort_view view{ cfg, data };

        core::array input{ data };
        std::thread sort_thread{ [&input, algo] { algo(input); } };
        auto& ev = core::event_manager::instance();

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
                    sound.sound_access(event.j);
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

        sound.quit();
    }
    catch(std::exception const& e) {
        TRACE("Exception thrown in main: {}", e.what());
    }
}
