#ifndef SORTVIS_EVENT_HPP
#define SORTVIS_EVENT_HPP
#pragma once

#include <cstddef>
#include <deque>
#include <list>
#include <mutex>
#include <utility>

namespace core {

using element_t = std::size_t;

enum class event_type
{
    access,
    swap,
    modify,
    compare
};

struct event_data
{
    event_type type{ event_type::access };
    element_t i{ 0 };
    element_t j{ 0 };
};

[[nodiscard]] auto operator==(event_data const& a, event_data const& b) noexcept -> bool;
[[nodiscard]] auto operator!=(event_data const& a, event_data const& b) noexcept -> bool;

class event_manager
{
    using locked_vector = std::pair<std::deque<event_data>, std::mutex>;
    static constexpr int s_max_events_per_block = 32;

private:
    std::list<locked_vector> m_events;

    event_manager();

public:
    event_manager(event_manager const&) = delete;
    event_manager(event_manager&&) = delete;
    ~event_manager() noexcept = default;

    auto operator=(event_manager const&) -> event_manager& = delete;
    auto operator=(event_manager &&) -> event_manager& = delete;

    [[nodiscard]] static auto instance() -> event_manager&;

    auto push(event_data const& event) -> void;
    [[nodiscard]] auto pop() -> event_data;
    [[nodiscard]] auto empty() noexcept -> bool;
};

struct normal_emitter
{
public:
    static auto on_access(element_t i) -> void;
    static auto on_swap(element_t i, element_t j) -> void;
    static auto on_comparison(element_t i, element_t j) -> void;
    static auto on_modify(element_t i, element_t value) -> void;
};

struct test_emitter
{
public:
    static auto on_access(element_t i) -> void;
    static auto on_swap(element_t i, element_t j) -> void;
    static auto on_comparison(element_t i, element_t j) -> void;
    static auto on_modify(element_t i, element_t value) -> void;
};

#ifdef SORTVIS_TESTING

using emitter_t = test_emitter;

#else

using emitter_t = normal_emitter;

#endif

} // namespace core

#endif // !SORTVIS_EVENT_HPP
