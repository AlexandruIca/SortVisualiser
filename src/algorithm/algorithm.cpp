#include "algorithm.hpp"
#include "event/event.hpp"
#include "log/log.hpp"

#include <queue>
#include <vector>

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

auto radix_sort_simple(core::array& data) -> void
{
    constexpr int Base = 10;
    std::array<std::queue<core::element_t>, Base> digits;
    core::element_t max = data.size();
    core::element_t pow{ 1 };

    while(max / pow > 0) {
        // for(int const num : v) {
        for(int i = 0; i < data.isize(); ++i) {
            digits.at((data[i].get() / pow) % Base).push(data[i].get_raw());
        }

        pow *= Base;

        core::element_t index = 0;
        for(core::element_t i = 0; i < Base; ++i) {
            while(!digits.at(i).empty()) {
                data[index++] = digits.at(i).front();
                data.modify(index - 1, data[index - 1].get_raw());
                digits.at(i).pop();
            }
        }
    }
}

auto median_of_three(core::array& v, int const left, int const right) -> int
{
    int const mid = left + (right - left) / 2;

    if(v[right] < v[left]) {
        v.swap_at(right, left);
    }
    if(v[mid] < v[left]) {
        v.swap_at(mid, left);
    }
    if(v[right] < v[mid]) {
        v.swap_at(right, mid);
    }

    return mid;
}

auto quicksort_impl(core::array& v, int const left, int const right) -> void
{
    if(right - left <= 0) {
        return;
    }
    if(right - left == 1) {
        if(v[right] < v[left]) {
            return v.swap_at(right, left);
        }
        return;
    }

    int i{ left };
    int j{ right };
    auto pivot = v[median_of_three(v, left, right)].get();

    while(i <= j) {
        while(v[i].get() < pivot) {
            ++i;
        }
        while(v[j].get() > pivot) {
            --j;
        }

        if(i <= j) {
            v.swap_at(i++, j--);
        }
    }

    if(j > left) {
        quicksort_impl(v, left, j);
    }
    if(i < right) {
        quicksort_impl(v, i, right);
    }
}

auto quicksort(core::array& data) -> void
{
    quicksort_impl(data, 0, data.isize() - 1);
    data.end();
}

auto merge(core::array& v, int const left, int const mid, int const right) -> void
{
    std::vector<core::element_t> tmp;
    tmp.reserve(std::size_t(right - left) + 1);

    int i = left;
    int j = mid + 1;

    while(i <= mid && j <= right) {
        if(v[i] < v[j]) {
            tmp.push_back(v[i++].get());
        }
        else {
            tmp.push_back(v[j++].get());
        }
    }

    while(i <= mid) {
        tmp.push_back(v[i++].get());
    }
    while(j <= right) {
        tmp.push_back(v[j++].get());
    }

    for(i = left; i <= right; ++i) {
        v[i] = tmp[std::size_t(i - left)];
        v.modify(core::element_t(i), tmp[std::size_t(i - left)]);
    }
}

auto merge_sort_impl(core::array& v, int const left, int const right) -> void
{
    if(left < right) {
        int const mid = left + (right - left) / 2;
        merge_sort_impl(v, left, mid);
        merge_sort_impl(v, mid + 1, right);
        merge(v, left, mid, right);
    }
}

auto merge_sort(core::array& data) -> void
{
    merge_sort_impl(data, 0, data.isize() - 1);
    data.end();
}

auto insertion_sort(core::array& data) -> void
{
    for(int i = 1; i < data.isize(); ++i) {
        int j = i - 1;
        while(j >= 0 && data[j] > data[j + 1]) {
            data.swap_at(j, j + 1);
            --j;
        }
    }

    data.end();
}

} // namespace core::algorithm
