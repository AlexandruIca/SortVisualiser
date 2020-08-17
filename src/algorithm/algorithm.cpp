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

} // namespace core::algorithm
