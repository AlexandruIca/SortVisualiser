#include "random.hpp"

#include <algorithm>
#include <random>

namespace core {

auto random_shuffle(std::vector<element_t>& data) -> void
{
    std::mt19937 rng{ std::random_device{}() };
    std::shuffle(data.begin(), data.end(), rng);
}

} // namespace core
