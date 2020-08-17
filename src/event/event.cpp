#include "event.hpp"
#include "log/log.hpp"

#include <algorithm>
#include <utility>

namespace core {

event_manager::event_manager()
{
    m_events.emplace_back();
}

auto event_manager::instance() -> event_manager&
{
    static event_manager mng;
    return mng;
}

auto event_manager::push(event_data const& event) -> void
{
    {
        std::scoped_lock<std::mutex> lock{ m_events.back().second };

        if(m_events.back().first.size() == s_max_events_per_block) {
            m_events.emplace_back();
        }
    }

    std::scoped_lock<std::mutex> lock{ m_events.back().second };
    auto& events = m_events.back().first;
    events.push_back(event);
}

auto event_manager::pop() -> event_data
{
    {
        std::scoped_lock<std::mutex> lock{ m_events.front().second };

        if(m_events.front().first.empty()) {
            m_events.pop_front();
        }
    }

    std::scoped_lock<std::mutex> lock{ m_events.front().second };
    auto const [type, i, j] = m_events.front().first.front();
    m_events.front().first.pop_front();
    return { type, i, j };
}

auto event_manager::empty() noexcept -> bool
{
    for(auto& event : m_events) {
        std::scoped_lock<std::mutex> lock{ event.second };

        if(!event.first.empty()) {
            return false;
        }
    }

    return true;
}

auto operator==(event_data const& a, event_data const& b) noexcept -> bool
{
    return a.type == b.type && a.i == b.i && a.j == b.j;
}

auto operator!=(event_data const& a, event_data const& b) noexcept -> bool
{
    return !(a == b);
}

auto normal_emitter::on_access(element_t const i) -> void
{
    TRACE("[Worker] Accessed at index {}", i);
    event_manager::instance().push({ event_type::access, i, 0 });
}

auto normal_emitter::on_swap(element_t const i, element_t const j) -> void
{
    TRACE("[Worker] Swapped at index ({}, {})", i, j);
    event_manager::instance().push({ event_type::swap, i, j });
}

auto normal_emitter::on_modify(element_t const i, element_t const value) -> void
{
    TRACE("[Worker] Modified at index {} with value {}", i, value);
    event_manager::instance().push({ event_type::modify, i, value });
}

auto normal_emitter::on_comparison(element_t const i, element_t const j) -> void
{
    TRACE("[Worker] Compared at index ({}, {})", i, j);
    event_manager::instance().push({ event_type::compare, i, j });
}

auto normal_emitter::on_end() -> void
{
    TRACE("[Worker] Ended sorting");
    event_manager::instance().push({ event_type::end, 0, 0 });
}

auto test_emitter::on_access(element_t const) -> void
{
}

auto test_emitter::on_swap(element_t const, element_t const) -> void
{
}

auto test_emitter::on_modify(element_t const, element_t const) -> void
{
}

auto test_emitter::on_comparison(element_t const, element_t const) -> void
{
}

auto test_emitter::on_end() -> void
{
}

array_value::array_value(element_t const init)
    : m_value{ init }
    , m_index{ 0 }
{
}

auto array_value::operator=(element_t const val) noexcept -> array_value&
{
    m_value = val;
    return *this;
}

auto array_value::set_index(element_t const index) const noexcept -> void
{
    m_index = index;
}

auto array_value::get() const noexcept -> element_t
{
    emitter_t::on_access(m_index);
    return m_value;
}

auto array_value::get_raw() const noexcept -> element_t
{
    return m_value;
}

auto array_value::index() const noexcept -> element_t
{
    return m_index;
}

#define OPERATOR(op)                                                                                                   \
    auto operator op(array_value const& a, array_value const& b) noexcept->bool                                        \
    {                                                                                                                  \
        emitter_t::on_comparison(a.index(), b.index());                                                                \
        return a.get_raw() op b.get_raw();                                                                             \
    }

OPERATOR(==)
OPERATOR(!=)
OPERATOR(<)
OPERATOR(<=)
OPERATOR(>=)
OPERATOR(>)

#undef OPERATOR

array::array(std::vector<element_t> const& input)
{
    m_data.reserve(input.size());

    for(element_t const val : input) {
        m_data.emplace_back(val);
    }
}

auto array::swap_at(element_t const i, element_t const j) -> void
{
    emitter_t::on_swap(i, j);
    std::swap(m_data[i], m_data[j]);
}

auto array::swap_at(int const i, int const j) -> void
{
    ASSERT(i >= 0);
    ASSERT(j >= 0);

    this->swap_at(static_cast<element_t>(i), static_cast<element_t>(j));
}

auto array::modify(element_t const i, element_t const val) const -> void
{
    emitter_t::on_modify(i, val);
    static_cast<void>(m_data); // ignore 'methd can be made static'
}

auto array::modify(int const i, int const val) const -> void
{
    ASSERT(i >= 0);

    this->modify(static_cast<element_t>(i), static_cast<element_t>(val));
}

auto array::end() -> void
{
    emitter_t::on_end();
    static_cast<void>(m_data); // ignore 'method can be made static'
}

auto array::operator[](element_t const index) noexcept -> array_value&
{
    emitter_t::on_access(index);
    auto& val = m_data[index];
    val.set_index(index);
    return val;
}

auto array::operator[](element_t const index) const noexcept -> array_value const&
{
    emitter_t::on_access(index);
    auto const& val = m_data[index];
    val.set_index(index);
    return val;
}

auto array::operator[](int const index) noexcept -> array_value&
{
    ASSERT(index >= 0);
    return this->operator[](static_cast<element_t>(index));
}

auto array::operator[](int const index) const noexcept -> array_value const&
{
    ASSERT(index >= 0);
    return this->operator[](static_cast<element_t>(index));
}

auto array::size() const noexcept -> std::size_t
{
    return m_data.size();
}

auto array::isize() const noexcept -> int
{
    return static_cast<int>(m_data.size());
}

auto array::is_sorted() const noexcept -> bool
{
    std::vector<element_t> data;
    data.reserve(m_data.size());

    for(auto const& val : m_data) {
        data.push_back(val.get_raw());
    }

    return std::is_sorted(data.begin(), data.end());
}

} // namespace core
