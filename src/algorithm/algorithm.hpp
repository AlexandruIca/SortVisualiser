#ifndef SORTVIS_ALGORITHM_HPP
#define SORTVIS_ALGORITHM_HPP
#pragma once

namespace core {

class array;

}

namespace core::algorithm {

auto bubble_sort(core::array& data) -> void;
auto radix_sort(core::array& data) -> void;

} // namespace core::algorithm

#endif // !SORTVIS_ALGORITHM_HPP
