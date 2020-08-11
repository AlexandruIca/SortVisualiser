#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest/doctest.h>

#include "event/event.hpp"

TEST_CASE("[Array] Array value")
{
    core::array_value v1{ 0 };
    core::array_value v2{ 0 };
    core::array_value v3{ 1 };

    REQUIRE(v1 == v2);
    REQUIRE(v1 != v3);
    REQUIRE(v2 != v3);
    REQUIRE(v1.get() == v2.get());
    REQUIRE(v1.get_raw() == v2.get_raw());
    REQUIRE(v1.index() == 0);
    REQUIRE(v2.index() == 0);
    REQUIRE(v3.index() == 0);

    v1.set_index(1);
    v2.set_index(2);
    v3.set_index(3);

    REQUIRE(v1.index() == 1);
    REQUIRE(v2.index() == 2);
    REQUIRE(v3.index() == 3);
}
