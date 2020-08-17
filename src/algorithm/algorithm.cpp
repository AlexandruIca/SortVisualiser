#include "algorithm.hpp"
#include "event/event.hpp"

namespace core::algorithm {

auto bubble_sort(core::array& data) -> void
{
    int sorted_offset = 0;
    bool is_sorted = false;

    do {
        is_sorted = true;

        for(int i = 1; i < data.isize() - sorted_offset; ++i) {
            if(data[i - 1] > data[i]) {
                data.swap_at(i - 1, i);
                is_sorted = false;
            }
        }

        if(!is_sorted) {
            ++sorted_offset;
        }
    } while(!is_sorted);

    data.end();
}

[[nodiscard]] constexpr auto nth_byte(element_t const x, element_t const n) -> element_t
{
    constexpr int byte_size = 8;
    constexpr int byte_max = 0xFF;
    return (x >> (n * byte_size)) & byte_max;
}

auto count_sort_helper(core::array const& input, core::array& tmp, core::element_t const byte) -> void
{
    constexpr element_t byte_size = 8;
    constexpr element_t max_count = 1 << byte_size; // 2 ^ 8

    auto byte_of = [byte](element_t const num) -> element_t { return nth_byte(num, byte); };

    std::array<element_t, max_count> counter{};
    std::array<element_t, max_count> idx{};

    counter.fill(0);

    for(element_t i = 0; i < input.size(); ++i) {
        ++counter.at(byte_of(input[i].get()));
    }

    idx[0] = 0;

    for(element_t i = 1; i < max_count; ++i) {
        idx.at(i) = idx.at(i - 1) + counter.at(i - 1);
    }

    for(element_t i = 0; i < input.size(); ++i) {
        auto const index = idx.at(byte_of(input[i].get()))++;
        auto const value = input[i].get_raw();
        tmp[index] = value;
        input.modify(index, value);
    }
}

auto radix_sort(core::array& data) -> void
{
    core::array tmp{ data }; // NOLINT
    element_t byte_index{ 0 };

    for(;;) {
        count_sort_helper(data, tmp, byte_index);
        count_sort_helper(tmp, data, byte_index + 1);
        count_sort_helper(data, tmp, byte_index + 2);
        count_sort_helper(tmp, data, byte_index + 3);

        if((byte_index += 4) >= sizeof(element_t)) {
            return;
        }
    }

    data.end();
}

} // namespace core::algorithm
