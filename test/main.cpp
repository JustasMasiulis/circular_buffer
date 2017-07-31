#define CATCH_CONFIG_MAIN
#include <circular_buffer.hpp>
#include "Catch/include/catch.hpp"
#include <algorithm>
#include <vector>
#include <numeric>
#include <list>

std::vector<int> gen_incremental_vector() {
    constexpr auto size = 512;
    std::vector<int> v(size);

    std::iota(v.begin(), v.end(), 0);

    return v;
}

const static auto inc_vec = gen_incremental_vector();

TEST_CASE("default construction")
{
    SECTION("const") {
        const jm::circular_buffer<int, 16> cb;
        REQUIRE(cb.size() == 0);

        REQUIRE(cb.max_size() == 16);

        REQUIRE(cb.begin() == cb.end());

        REQUIRE(cb.cbegin() == cb.cend());

        REQUIRE(cb.rbegin() == cb.rend());

        REQUIRE(cb.crbegin() == cb.crend());
    }

    SECTION("non const")
    {
        jm::circular_buffer<int, 16> cb;
        REQUIRE(cb.size() == 0);

        REQUIRE(cb.max_size() == 16);

        REQUIRE(cb.begin() == cb.end());

        REQUIRE(cb.cbegin() == cb.cend());

        REQUIRE(cb.rbegin() == cb.rend());

        REQUIRE(cb.crbegin() == cb.crend());
    }
}

TEST_CASE("push_back works correctly")
{
    jm::circular_buffer<int, 16> cb;

    for (auto i : inc_vec) {
        cb.push_back(i);
        REQUIRE(cb.back() == i);
        REQUIRE(*cb.end() == i);
        auto front = cb.front();
        for (auto v : cb)
            REQUIRE(v == front++);
    }

    REQUIRE(cb.size() == cb.max_size());
}

TEST_CASE("push_front works correctly")
{
    jm::circular_buffer<int, 16> cb;

    for (auto i : inc_vec) {
        cb.push_front(i);
        REQUIRE(cb.front() == i);
        REQUIRE(*cb.begin() == i);
        for (auto v : cb)
            REQUIRE(v == i--);
    }

    REQUIRE(cb.size() == cb.max_size());
}
