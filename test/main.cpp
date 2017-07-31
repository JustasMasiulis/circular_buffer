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

jm::circular_buffer<int, 16> gen_filled_cb(int size = 16)
{
    jm::circular_buffer<int, 16> cb;
    for (int i = 0; i < size; ++i)
        cb.push_back(i);

    return cb;
}

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

TEST_CASE("copy construction")
{
    auto cb = gen_filled_cb(15);

    auto other = cb;

    REQUIRE(std::equal(cb.begin(), cb.end(), other.begin()));
}

TEST_CASE("move construction")
{
    auto cb = gen_filled_cb(15);

    auto temp = cb;

    auto other = std::move(temp);

    REQUIRE(std::equal(cb.begin(), cb.end(), other.begin()));
}

TEST_CASE("copy assignment")
{
    auto cb = gen_filled_cb(15);

    decltype(cb) other;
    other = cb;

    REQUIRE(std::equal(cb.begin(), cb.end(), other.begin()));
}

TEST_CASE("move assignment")
{
    auto cb = gen_filled_cb(15);

    auto temp(cb);

    decltype(cb) other;
    other = std::move(temp);

    REQUIRE(std::equal(cb.begin(), cb.end(), other.begin()));
}

TEST_CASE("initializer_list construction")
{
    REQUIRE_THROWS(void(jm::circular_buffer<int, 4>{ 1, 2, 3, 5, 6 }));
    jm::circular_buffer<int, 4> buf{ { 1, 2, 3, 5 } };
}

TEST_CASE("push_back")
{
    jm::circular_buffer<int, 16> cb;

    for (auto i : inc_vec) {
        cb.push_back(i);
        REQUIRE(cb.back() == i);
        REQUIRE(*--cb.end() == i);
        auto front = cb.front();
        for (auto v : cb)
            REQUIRE(v == front++);
    }

    REQUIRE(cb.size() == cb.max_size());
}

TEST_CASE("push_front")
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

TEST_CASE("circular_buffer_iterator complies to Iterator")
{
    using cbt = jm::circular_buffer<int, 4>;
    cbt cb;
    cb.push_back(1);
    cb.push_back(2);

        auto r = cb.begin();
        // CopyConstructible
        {
            // MoveConstructible 
            {
                auto v = cb.begin();

                REQUIRE(v == cb.begin());
                REQUIRE(cbt::iterator(cb.begin()) == cb.begin());
            }
            
            auto v = cb.begin();
            {
                auto u = r;
                REQUIRE(u == v);
                REQUIRE(v == r);
            }
            {
                REQUIRE(cbt::iterator(v) == v);
                REQUIRE(v == r);
            }
             
        }

        // CopyAssignable
        {
            // MoveAssignable 
            {
                auto t = cb.begin();
                t = cb.begin();
                static_assert(std::is_same<decltype(t = cb.begin()), cbt::iterator&>::value
                              , "t = cb.begin() doesnt return T&");
                
                REQUIRE((t = cb.end()) == t);
            }
            {
                auto t = cb.begin();
                auto v = cb.end();
                static_assert(std::is_same<decltype(t = v), cbt::iterator&>::value
                              , "t = cb.begin() doesnt return T&");

                REQUIRE((t = v) == t);
            }
        }

        // Swappable
        {
            using std::swap;
            auto u = cb.begin();
            auto t = cb.end();

            swap(u, t);
            REQUIRE(u == cb.end());
            REQUIRE(t == cb.begin());

            swap(t, u);
            REQUIRE(u == cb.begin());
            REQUIRE(t == cb.end());
        }

        {
            using value_type = cbt::iterator::value_type; // has all the typedefs
            using difference_type = cbt::iterator::difference_type;
            using reference = cbt::iterator::reference;
            using pointer = cbt::iterator::pointer;
            using iterator_category = cbt::iterator::iterator_category;
        }
        *r; // r is dereferenceable
        ++r; // r is incrementable
        static_assert(std::is_same<decltype(++r), decltype(r)&>::value, "++it doesnt return It&");
}

TEST_CASE("circular_buffer_iterator complies to InputIterator")
{
    using cbt = jm::circular_buffer<int, 4>;
    cbt cb;
    cb.push_back({ 1 });
    cb.push_back({ 2 });

    // EqualityComparable
    {
        auto a = cb.begin();
        auto b = cb.begin();
        auto c = b;

        REQUIRE(a == a);
        REQUIRE(a == b);
        REQUIRE(b == a);
        REQUIRE(a == c);

        static_assert(std::is_same<bool, decltype(a == b)>::value, "a==b not bool");
    }

    auto i = cb.begin();
    auto j = cb.end();

    REQUIRE(((void)*i, *i) == *i);

    REQUIRE((i != j) == (!(i == j)));

    /// TODO finish this...
}

TEST_CASE("circular_buffer_iterator complies to ForwardIterator")
{
    using cbt = jm::circular_buffer<int, 4>;
    using cbi = cbt::iterator;
    {
        cbi u;
    }
    {
        cbi u{};
    }
    {
        cbi();
    }
    {
        cbi{};
    }
    
    // TODO finish this
}
