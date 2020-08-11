#include "event.hpp"

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

} // namespace core
