#ifndef SORTVIS_LOG_HPP
#define SORTVIS_LOG_HPP
#pragma once

#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>

#include <array>
#include <cstdlib>
#include <memory>

namespace core {

struct logger
{
private:
    std::shared_ptr<spdlog::logger> m_logger;

    logger();

public:
    logger(logger const&) = delete;
    logger(logger&&) = delete;
    ~logger() noexcept = default;

    auto operator=(logger const&) = delete;
    auto operator=(logger&&) = delete;

    [[nodiscard]] static auto instance() -> logger&;
    [[nodiscard]] static auto get() -> std::shared_ptr<spdlog::logger>&;
};

} // namespace core

#ifdef SORTVIS_DEBUG

#define TRACE(...) ::core::logger::get()->trace(__VA_ARGS__)
#define INFO(...) ::core::logger::get()->info(__VA_ARGS__)
#define WARN(...) ::core::logger::get()->warn(__VA_ARGS__)
#define ERROR(...) ::core::logger::get()->error(__VA_ARGS__)
#define FATAL(...) ::core::logger::get()->critical(__VA_ARGS__)

#define ASSERT(...)                                                                                                    \
    if(!(__VA_ARGS__)) {                                                                                               \
        FATAL("Assertion failed at {}[{}]: {}", __FILE__, __LINE__, #__VA_ARGS__);                                     \
        std::exit(EXIT_FAILURE);                                                                                       \
    }                                                                                                                  \
    static_cast<void>(0)

#else

#define TRACE(...) static_cast<void>(0)
#define INFO(...) static_cast<void>(0)
#define WARN(...) static_cast<void>(0)
#define ERROR(...) static_cast<void>(0)
#define FATAL(...) static_cast<void>(0)
#define ASSERT(...) static_cast<void>(0)

#endif

#endif // !SORTVIS_LOG_HPP
