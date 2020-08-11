#include "algorithm.hpp"
#include "event/event.hpp"

namespace core::algorithm {

auto bubble_sort(core::array& data) -> void
{
    bool is_sorted = false;

    do {
        is_sorted = true;

        for(int i = 1; i < data.isize(); ++i) {
            if(data[i - 1] > data[i]) {
                data.swap_at(i - 1, i);
                is_sorted = false;
            }
        }
    } while(!is_sorted);
}

} // namespace core::algorithm
