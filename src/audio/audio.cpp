#include "audio.hpp"
#include "log/log.hpp"

#include <SDL.h>

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <limits>
#include <mutex>
#include <string>
#include <vector>

// Pretty much all of this is taken from https://panthema.net/2013/sound-of-sorting/sound-of-sorting-0.6.5/src/
namespace audio {

audio_manager::audio_manager()
{
    if(SDL_InitSubSystem(SDL_INIT_AUDIO) != 0) {
        TRACE("Couldn't initialize SDL_Audio: {}", SDL_GetError());
    }

    SDL_AudioSpec want;
    want.freq = audio_manager::get_sample_rate();
    want.format = AUDIO_S16SYS;
    want.channels = 1;
    want.samples = 4'096; // NOLINT
    want.callback = &audio_manager::sound_callback;
    want.userdata = nullptr;

    m_audio_device = SDL_OpenAudioDevice(nullptr, 0, &want, nullptr, SDL_AUDIO_ALLOW_FORMAT_CHANGE);

    if(m_audio_device == 0) {
        TRACE("Couldn't open audio: {}", SDL_GetError());
    }

    SDL_PauseAudioDevice(m_audio_device, 0);
}

auto audio_manager::instance() -> audio_manager&
{
    static audio_manager amng;
    return amng;
}

auto audio_manager::quit() const noexcept -> void
{
    SDL_CloseAudioDevice(m_audio_device);
}

class oscillator
{
private:
    double m_freq;
    std::size_t m_tstart;
    std::size_t m_tend;
    std::size_t m_duration;

    static constexpr std::size_t s_default_duration = audio_manager::get_sample_rate() / 8;

public:
    oscillator() = default;
    oscillator(oscillator const&) = default;
    oscillator(oscillator&&) noexcept = default;
    ~oscillator() noexcept = default;

    oscillator(double const freq, std::size_t const tstart, std::size_t const duration = s_default_duration)
        : m_freq{ freq }
        , m_tstart{ tstart }
        , m_tend{ m_tstart + duration }
        , m_duration{ duration }
    {
    }

    auto operator=(oscillator const&) -> oscillator& = default;
    auto operator=(oscillator&&) noexcept -> oscillator& = default;

    [[nodiscard]] static auto wave_sin(double const x) noexcept -> double
    {
        return std::sin(x * 2 * M_PI);
    }

    [[nodiscard]] static auto wave_sin3(double const x) noexcept -> double
    {
        auto const s = std::sin(x * 2 * M_PI);
        return s * s * s;
    }

    [[nodiscard]] static auto wave_triangle(double x) noexcept -> double
    {
        x = std::fmod(x, 1.0);

        constexpr double threshold1 = 0.25;
        constexpr double threshold2 = 0.75;

        constexpr double p2 = 2.0;
        constexpr double p4 = 4.0;

        if(x <= threshold1) {
            return p4 * x;
        }
        if(x <= threshold2) {
            return p2 - p4 * x;
        }
        return p4 * x - p4;
    }

    [[nodiscard]] static auto wave(double const x) noexcept -> double
    {
        return wave_triangle(x);
    }

    [[nodiscard]] auto envelope(std::size_t const i) const noexcept -> double
    {
        auto x = double(i) / double(m_duration);

        if(x > 1.0) {
            x = 1.0;
        }

        constexpr double attack = 0.025;
        constexpr double decay = 0.1;
        constexpr double sustain = 0.9;
        constexpr double release = 0.3;

        if(x < attack) {
            return 1.0 / attack * x;
        }

        if(x < attack + decay) {
            return 1.0 - (x - attack) / decay * (1.0 - sustain);
        }

        if(x < 1.0 - release) {
            return sustain;
        }

        return sustain / release * (1.0 - x);
    }

    auto mix(double* data, int const size, std::size_t const p) const noexcept -> void
    {
        for(int i = 0; i < size; ++i) {
            if(int(p) + i < int(m_tstart)) {
                continue;
            }

            if(int(p) + i >= int(m_tend)) {
                break;
            }

            auto trel = std::size_t(int(p) + i - int(m_tstart));
            data[i] += // NOLINT
                this->envelope(trel) * wave(double(trel) / audio_manager::get_sample_rate() * m_freq);
        }
    }

    [[nodiscard]] auto tstart() const noexcept -> std::size_t
    {
        return m_tstart;
    }

    [[nodiscard]] auto is_done(std::size_t const p) const noexcept -> bool
    {
        return (p >= m_tend);
    }
};

static auto s_osclist() -> std::vector<oscillator>&
{
    static std::vector<oscillator> list;
    return list;
}

static std::size_t s_pos = 0;

static auto add_oscillator(double const freq, std::size_t const p, std::size_t const pstart, std::size_t const duration)
    -> void
{
    std::size_t oldest = 0;
    std::size_t toldest = std::numeric_limits<std::size_t>::max();

    for(std::size_t i = 0; i < s_osclist().size(); ++i) {
        if(s_osclist()[i].is_done(p)) {
            s_osclist()[i] = oscillator{ freq, pstart, duration };
            return;
        }

        if(s_osclist()[i].tstart() < toldest) {
            oldest = i;
            toldest = s_osclist()[i].tstart();
        }
    }

    if(s_osclist().size() < audio_manager::get_max_oscillators()) {
        s_osclist().emplace_back(freq, pstart, duration);
    }
    else {
        s_osclist()[oldest] = oscillator{ freq, pstart, duration };
    }
}

static auto s_access_list() -> std::vector<std::size_t>&
{
    static std::vector<std::size_t> v;
    return v;
}

static std::mutex s_mutex_access_list;

auto audio_manager::sound_access(std::size_t const i) const -> void
{
    if(!m_sound_on) {
        return;
    }

    std::scoped_lock<std::mutex> lock{ s_mutex_access_list };
    s_access_list().push_back(i);
}

static auto array_index_to_freq(double const index) noexcept -> double
{
    auto const result = 120 + 1'200 * (index * index);
    return result;
}

auto audio_manager::sound_reset() -> void
{
    std::scoped_lock<std::mutex> lock{ s_mutex_access_list };
    s_pos = 0;
    s_osclist().clear();
}

auto audio_manager::sound_callback(void*, Uint8* stream, int len) -> void
{
    if(!audio_manager::instance().m_sound_on) {
        std::memset(stream, 0, std::size_t(len));
        return;
    }

    std::size_t& p = s_pos;
    std::int16_t* data = reinterpret_cast<std::int16_t*>(stream); // NOLINT
    std::size_t size = std::size_t(len) / sizeof(std::int16_t);

    {
        std::scoped_lock<std::mutex> lock{ s_mutex_access_list };
        auto const pscale = double(size) / double(s_access_list().size());

        for(std::size_t i = 0; i < s_access_list().size(); ++i) {
            double relindex = double(s_access_list()[i]) / double(audio_manager::instance().m_max_number);
            double freq = array_index_to_freq(relindex);
            add_oscillator(freq,
                           p,
                           static_cast<std::size_t>(double(p) + double(i) * pscale),
                           static_cast<std::size_t>(audio_manager::instance().m_delay_sec / 1'000.0 * // NOLINT
                                                    audio_manager::sound_sustain() * audio_manager::get_sample_rate()));
        }

        s_access_list().clear();
    }

    std::vector<double> wave(size, 0.0);
    std::size_t wavecount = 0;

    for(auto const& osc : s_osclist()) {
        if(!osc.is_done(p)) {
            osc.mix(wave.data(), int(size), p);
            ++wavecount;
        }
    }

    if(wavecount == 0) {
        std::memset(stream, 0, std::size_t(len));
    }
    else {
        double vol = *std::max_element(wave.begin(), wave.end());
        static double oldvol = 1.0;

        if(vol > oldvol) {
        }
        else {
            constexpr double factor = 0.9;
            vol = factor * oldvol;
        }

        for(std::size_t i = 0; i < size; ++i) {
            auto v =
                std::int32_t(24'000.0 * wave[i] / (oldvol + (vol - oldvol) * (double(i) / double(size)))); // NOLINT

            constexpr int threshold_max = 32'200;
            constexpr int threshold_min = -threshold_max;

            if(v > threshold_max) {
                v = threshold_max;
            }
            if(v < threshold_min) {
                v = threshold_min;
            }

            data[i] = std::int16_t(v); // NOLINT
        }

        oldvol = vol;
    }

    p += size;
}

} // namespace audio
