#ifndef SORTVIS_AUDIO_HPP
#define SORTVIS_AUDIO_HPP
#pragma once

#include <SDL.h>

#include <cstddef>

namespace audio {

struct audio_manager
{
private:
    SDL_AudioDeviceID m_audio_device;
    bool m_sound_on = true;
    std::size_t m_max_number = 0;
    double m_delay_sec = 0.0;

    static constexpr double s_default_sound_sustain = 2.0;
    double m_sound_sustain = s_default_sound_sustain;

    static constexpr int s_max_num_oscillators = 512;
    static constexpr int s_sample_rate = 44'100;

    audio_manager();

public:
    audio_manager(audio_manager const&) = delete;
    audio_manager(audio_manager&&) noexcept = delete;
    ~audio_manager() noexcept = default;

    auto operator=(audio_manager const&) = delete;
    auto operator=(audio_manager&&) noexcept = delete;

    [[nodiscard]] static auto instance() -> audio_manager&;
    auto quit() -> void;

    constexpr static auto get_sample_rate() -> int
    {
        return s_sample_rate;
    }

    constexpr static auto get_max_oscillators() -> int
    {
        return s_max_num_oscillators;
    }

    [[nodiscard]] inline auto sound_on() const noexcept -> bool
    {
        return m_sound_on;
    }

    inline auto turn_sound_on() noexcept -> void
    {
        m_sound_on = true;
    }

    inline auto turn_sound_off() noexcept -> void
    {
        m_sound_on = false;
    }

    inline auto set_delay(double const sec) noexcept -> void
    {
        m_delay_sec = sec;
    }

    inline static auto sound_sustain() -> double
    {
        return audio_manager::instance().m_sound_sustain;
    }

    inline auto set_max(std::size_t const elem) noexcept -> void
    {
        m_max_number = elem;
    }

    auto sound_access(std::size_t i) const -> void;
    static auto sound_reset() -> void;
    static auto sound_callback(void* udata, Uint8* stream, int len) -> void;
};

} // namespace audio

#endif // !SORTVIS_AUDIO_HPP
