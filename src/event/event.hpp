#ifndef SORTVIS_EVENT_HPP
#define SORTVIS_EVENT_HPP
#pragma once

#include <cstddef>
#include <deque>
#include <list>
#include <mutex>
#include <utility>
#include <vector>

namespace core {

using element_t = std::size_t;

enum class event_type
{
    access,
    swap,
    modify,
    compare,
    end
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
    static auto on_end() -> void;
};

struct test_emitter
{
public:
    static auto on_access(element_t i) -> void;
    static auto on_swap(element_t i, element_t j) -> void;
    static auto on_comparison(element_t i, element_t j) -> void;
    static auto on_modify(element_t i, element_t value) -> void;
    static auto on_end() -> void;
};

#ifdef SORTVIS_TESTING

using emitter_t = test_emitter;

#else

using emitter_t = normal_emitter;

#endif

class array_value
{
private:
    element_t m_value;
    mutable element_t m_index; // where the value comes from

public:
    array_value() noexcept = default;
    array_value(array_value const&) noexcept = default;
    array_value(array_value&&) noexcept = default;
    ~array_value() noexcept = default;

    explicit array_value(element_t init);

    auto operator=(array_value const&) noexcept -> array_value& = default;
    auto operator=(array_value&&) noexcept -> array_value& = default;

    auto operator=(element_t val) noexcept -> array_value&;

    auto set_index(element_t index) const noexcept -> void;

    [[nodiscard]] auto get() const noexcept -> element_t;
    [[nodiscard]] auto get_raw() const noexcept -> element_t;
    [[nodiscard]] auto index() const noexcept -> element_t;
};

[[nodiscard]] auto operator==(array_value const& a, array_value const& b) noexcept -> bool;
[[nodiscard]] auto operator!=(array_value const& a, array_value const& b) noexcept -> bool;
[[nodiscard]] auto operator<(array_value const& a, array_value const& b) noexcept -> bool;
[[nodiscard]] auto operator<=(array_value const& a, array_value const& b) noexcept -> bool;
[[nodiscard]] auto operator>=(array_value const& a, array_value const& b) noexcept -> bool;
[[nodiscard]] auto operator>(array_value const& a, array_value const& b) noexcept -> bool;

class array
{
private:
    std::vector<array_value> m_data;

public:
    array() noexcept = default;
    array(array const&) = default;
    array(array&&) noexcept = default;
    ~array() noexcept = default;

    explicit array(std::vector<element_t> const& input);

    auto operator=(array const&) noexcept -> array& = default;
    auto operator=(array&&) noexcept -> array& = default;

    auto swap_at(element_t i, element_t j) -> void;
    auto swap_at(int i, int j) -> void;
    auto modify(element_t i, element_t val) const -> void;
    auto modify(int i, int val) const -> void;
    auto end() -> void;

    [[nodiscard]] auto operator[](element_t index) noexcept -> array_value&;
    [[nodiscard]] auto operator[](element_t index) const noexcept -> array_value const&;
    [[nodiscard]] auto operator[](int index) noexcept -> array_value&;
    [[nodiscard]] auto operator[](int index) const noexcept -> array_value const&;

    [[nodiscard]] auto size() const noexcept -> std::size_t;
    [[nodiscard]] auto isize() const noexcept -> int;

    [[nodiscard]] auto is_sorted() const noexcept -> bool;
};

} // namespace core

#endif // !SORTVIS_EVENT_HPP
