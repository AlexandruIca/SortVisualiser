#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest/doctest.h>

#include "algorithm/algorithm.hpp"
#include "algorithm/random.hpp"
#include "event/event.hpp"

#include <algorithm>
#include <array>
#include <numeric>
#include <random>
#include <unordered_map>
#include <vector>

template<std::size_t N>
[[nodiscard]] auto to_array(core::element_t const (&arr)[N]) -> std::array<core::element_t, N>
{
    std::array<core::element_t, N> result{};

    for(std::size_t i = 0; i < N; ++i) {
        result.at(i) = arr[i];
    }

    return result;
}

struct sort_data
{
private:
    [[nodiscard]] static auto generate(core::element_t const count) -> std::vector<core::element_t>
    {
        std::vector<core::element_t> result;

        result.resize(count);
        std::iota(result.begin(), result.end(), 1);
        core::random_shuffle(result);

        return result;
    }

    std::unordered_map<core::element_t, std::vector<core::element_t>> m_cache;

    sort_data() = default;

public:
    sort_data(sort_data const&) = delete;
    sort_data(sort_data&&) = delete;
    ~sort_data() noexcept = default;

    auto operator=(sort_data const&) = delete;
    auto operator=(sort_data&&) = delete;

    [[nodiscard]] static auto instance() -> sort_data&
    {
        static sort_data inst;
        return inst;
    }

    [[nodiscard]] static auto for_size(core::element_t const size) -> std::vector<core::element_t> const&
    {
        auto& cache = sort_data::instance().m_cache;

        if(cache.find(size) != cache.end()) {
            return cache[size];
        }

        cache[size] = generate(size);
        return cache[size];
    }
};

TEST_CASE("[Algorithm] Bubble Sort")
{
    auto const sizes = to_array({ 5, 10, 100, 250 });

    for(auto const size : sizes) {
        core::array data{ sort_data::for_size(size) };
        core::algorithm::bubble_sort(data);
        REQUIRE(data.is_sorted());
    }
}

TEST_CASE("[Algorithm] Radix Sort")
{
    auto const sizes = to_array({ 5, 10, 100, 250, 1'000, 5'000, 10'000 });

    for(auto const size : sizes) {
        core::array data{ sort_data::for_size(size) };
        core::algorithm::radix_sort(data);
        REQUIRE(data.is_sorted());
    }
}

TEST_CASE("[Algorithm] Simple Radix Sort")
{
    auto const sizes = to_array({ 5, 10, 100, 250, 1'000, 5'000, 10'000 });

    for(auto const size : sizes) {
        core::array data{ sort_data::for_size(size) };
        core::algorithm::radix_sort_simple(data);
        REQUIRE(data.is_sorted());
    }
}
