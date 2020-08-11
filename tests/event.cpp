#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest/doctest.h>

#include "event/event.hpp"

#include <chrono>
#include <mutex>
#include <random>
#include <thread>
#include <vector>

TEST_CASE("[EventData] operator==/!=")
{
    core::event_data const ev1{ core::event_type::compare, 1, 1 };
    core::event_data const ev2{ core::event_type::compare, 1, 1 };
    core::event_data const ev3{ core::event_type::access, 1, 1 };

    REQUIRE(ev1 == ev2);
    REQUIRE(ev1 != ev3);
    REQUIRE(ev2 != ev3);
}

TEST_CASE("[Event] Check if order of pushed/popped events is the same")
{
    std::vector<core::event_data> const events = {
        { core::event_type::access, 0, 0 }, { core::event_type::compare, 1, 1 }, { core::event_type::modify, 2, 2 },
        { core::event_type::swap, 3, 3 },   { core::event_type::compare, 4, 4 }, { core::event_type::modify, 5, 5 },
        { core::event_type::access, 6, 6 }, { core::event_type::modify, 7, 7 },  { core::event_type::swap, 8, 8 }
    };
    std::vector<core::event_data> popped_events;

    popped_events.reserve(events.size());

    std::thread t{ [&events] {
        for(auto const& ev : events) {
            core::event_manager::instance().push(ev);
        }
    } };

    auto& mng = core::event_manager::instance();

    using namespace std::chrono_literals;
    std::this_thread::sleep_for(15ms);

    while(!mng.empty()) {
        popped_events.push_back(mng.pop());
    }

    t.join();

    REQUIRE(events == popped_events);
    REQUIRE(mng.empty());
}

TEST_CASE("[Event] Check if order of pushed/popped events is the same for many events")
{
    std::mt19937 rng{ std::random_device{}() };
    std::uniform_int_distribution<std::size_t> dist{ 0, 1'000 }; // NOLINT

    auto generate_event = [&rng, &dist]() -> core::event_data {
        auto const random = dist(rng) % 4;
        core::event_type type;

        switch(random) {
        case 0: {
            type = core::event_type::access;
            break;
        }
        case 1: {
            type = core::event_type::swap;
            break;
        }
        case 2: {
            type = core::event_type::modify;
            break;
        }
        case 3: {
            type = core::event_type::compare;
            break;
        }
        default: {
            type = core::event_type::access;
            break;
        }
        }

        return { type, dist(rng), dist(rng) };
    };

    auto generate_events = [&generate_event]() -> std::vector<core::event_data> {
        constexpr std::size_t num_events = 1'000;
        std::vector<core::event_data> result;

        result.reserve(num_events);

        for(std::size_t i = 0; i < num_events; ++i) {
            result.push_back(generate_event());
        }

        return result;
    };

    std::vector<core::event_data> const events = generate_events();
    std::vector<core::event_data> popped_events;

    popped_events.reserve(events.size());

    std::thread t{ [&events] {
        for(auto const& ev : events) {
            core::event_manager::instance().push(ev);
        }
    } };

    auto& mng = core::event_manager::instance();

    using namespace std::chrono_literals;
    std::this_thread::sleep_for(15ms);

    while(!mng.empty()) {
        popped_events.push_back(mng.pop());
    }

    t.join();

    REQUIRE(events == popped_events);
    REQUIRE(mng.empty());
}
