#include "log.hpp"

namespace core {

logger::logger()
{
    std::array<spdlog::sink_ptr, 2> sinks = { { std::make_shared<spdlog::sinks::stdout_color_sink_mt>(),
                                                std::make_shared<spdlog::sinks::basic_file_sink_mt>(
                                                    "SortVisualizer.txt", /* truncate: */ true) } };

    m_logger = std::make_shared<spdlog::logger>("SortVisualizer", sinks.begin(), sinks.end());
    spdlog::register_logger(m_logger);
    m_logger->set_level(spdlog::level::trace);
    m_logger->flush_on(spdlog::level::trace);
}

auto logger::instance() -> logger&
{
    static logger inst;
    return inst;
}

auto logger::get() -> std::shared_ptr<spdlog::logger>&
{
    return instance().m_logger;
}

} // namespace core
