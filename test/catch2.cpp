#define CATCH_CONFIG_MAIN
#define JM_CIRCULAR_BUFFER_CXX14
#include <circular_buffer.hpp>
#include <catch2/catch.hpp>

#include <numeric>
#include <vector>
#include <atomic>

std::uint64_t num_constructions = 0;
std::uint64_t num_deletions     = 0;

struct leak_checker {
    leak_checker() { ++num_constructions; }

    ~leak_checker() { ++num_deletions; }

    leak_checker(const leak_checker&) { ++num_constructions; }

    leak_checker(leak_checker&&) { ++num_constructions; }

    leak_checker& operator=(const leak_checker&) = default;
    leak_checker& operator=(leak_checker&&) noexcept = default;

    std::vector<float> aa{1.f, 2.f, 4.f};
};


std::vector<int> gen_incremental_vector()
{
    constexpr auto   size = 512;
    std::vector<int> v(size);

    std::iota(v.begin(), v.end(), 0);

    return v;
}

const static auto inc_vec = gen_incremental_vector();

jm::static_circular_buffer<int, 16> gen_filled_cb(int size = 16)
{
    jm::static_circular_buffer<int, 16> cb;
    for(int i = 0; i < size; ++i)
        cb.push_back(i);

    return cb;
}

jm::dynamic_circular_buffer<int> dynamic_gen_filled_cb(int size = 16)
{
  jm::dynamic_circular_buffer<int> cb(size);
  for (int i = 0; i < size; ++i)
    cb.push_back(i);

  return cb;
}

TEST_CASE("quick test for leaks")
{
    {
        jm::static_circular_buffer<leak_checker, 2> buf;
        for(int i = 0; i < 128; ++i)
            buf.push_back({});
        jm::static_circular_buffer<leak_checker, 7> buf2(buf.begin(), buf.end());
        jm::static_circular_buffer<leak_checker, 2> buf3{ {}, {} };
        buf = buf3;
        buf2.clear();
    }
    INFO("constructions: " << num_constructions << "deletions: " << num_deletions);
    REQUIRE(num_constructions == num_deletions);
}

TEST_CASE("dynamic quick test for leaks")
{
  {
    jm::dynamic_circular_buffer<leak_checker> buf;
    buf.reserve(2);
    for (int i = 0; i < 128; ++i)
      buf.push_back({});
    jm::dynamic_circular_buffer<leak_checker> buf2(buf.begin(), buf.end());
    jm::dynamic_circular_buffer<leak_checker> buf3{ {}, {} };
    buf = buf3;
    buf2.clear();
  }
  INFO("constructions: " << num_constructions << "deletions: " << num_deletions);
  REQUIRE(num_constructions == num_deletions);
}

TEST_CASE("default construction")
{
    SECTION("const")
    {
        const jm::static_circular_buffer<int, 16> cb;
        REQUIRE(cb.size() == 0);

        REQUIRE(cb.max_size() == 16);

        REQUIRE(cb.begin() == cb.end());

        REQUIRE(cb.cbegin() == cb.cend());

        REQUIRE(cb.rbegin() == cb.rend());

        REQUIRE(cb.crbegin() == cb.crend());
    }

    SECTION("non const")
    {
        jm::static_circular_buffer<int, 16> cb;
        REQUIRE(cb.size() == 0);

        REQUIRE(cb.max_size() == 16);

        REQUIRE(cb.begin() == cb.end());

        REQUIRE(cb.cbegin() == cb.cend());

        REQUIRE(cb.rbegin() == cb.rend());

        REQUIRE(cb.crbegin() == cb.crend());
    }
}

TEST_CASE("dynamic default construction")
{
  SECTION("const")
  {
    const jm::dynamic_circular_buffer<int> cb(16);
    REQUIRE(cb.size() == 16);

    REQUIRE(cb.max_size() == 16);

    REQUIRE(cb.begin() != cb.end());

    REQUIRE(cb.cbegin() != cb.cend());

    REQUIRE(cb.rbegin() != cb.rend());

    REQUIRE(cb.crbegin() != cb.crend());
  }

  SECTION("non const")
  {
    jm::dynamic_circular_buffer<int> cb(16);

    REQUIRE(cb.size() == 16);

    REQUIRE(cb.max_size() == 16);

    REQUIRE(cb.begin() != cb.end());

    REQUIRE(cb.cbegin() != cb.cend());

    REQUIRE(cb.rbegin() != cb.rend());

    REQUIRE(cb.crbegin() != cb.crend());
  }
}

TEST_CASE("copy construction")
{
    auto cb = gen_filled_cb(15);

    auto other = cb;

    REQUIRE(std::equal(cb.begin(), cb.end(), other.begin()));
}
TEST_CASE("dynamic copy construction")
{
  auto cb = dynamic_gen_filled_cb(15);

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

TEST_CASE("dynamic move construction")
{
  auto cb = dynamic_gen_filled_cb(15);

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

TEST_CASE("dynamic copy assignment")
{
  auto cb = dynamic_gen_filled_cb(15);

  decltype(cb) other(15);
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

TEST_CASE("dynamic move assignment")
{
  auto cb = dynamic_gen_filled_cb(15);

  auto temp(cb);

  decltype(cb) other;
  other = std::move(temp);

  REQUIRE(std::equal(cb.begin(), cb.end(), other.begin()));
}
//
#ifndef JM_CIRCULAR_BUFFER_CXX_OLD
TEST_CASE("initializer_list construction")
{
    REQUIRE_THROWS(void(jm::static_circular_buffer<int, 4>{ 1, 2, 3, 5, 6 }));
    jm::static_circular_buffer<int, 4> buf{ { 1, 2, 3, 5 } };
}

TEST_CASE("dynamic initializer_list construction")
{
  //REQUIRE_THROWS(void(jm::dynamic_circular_buffer<int>{ 1, 2, 3, 5, 6 }));
  //jm::dynamic_circular_buffer<int> buf{ { 1, 2, 3, 5 } };
}
//
TEST_CASE("iterators construction")
{
    {
        auto cb = gen_filled_cb(15);
        REQUIRE_THROWS(void(jm::static_circular_buffer<int, 4>{ cb.begin(), cb.end() }));

        jm::static_circular_buffer<int, 16> cb2(cb.begin(), cb.end());

        REQUIRE(std::equal(cb.begin(), cb.end(), cb2.begin()));
        REQUIRE(cb.size() == cb2.size());
    }

    jm::static_circular_buffer<int, 4> buf1{ 1, 2, 3, 4 };
    jm::static_circular_buffer<int, 4> buf2{ buf1.begin(), buf1.end() };

    REQUIRE(std::equal(buf1.begin(), buf1.end(), buf2.begin()));
    REQUIRE(buf1.size() == buf2.size());
}
TEST_CASE("dynamic iterators construction")
{
  {
    auto cb = dynamic_gen_filled_cb(15);
    //REQUIRE_THROWS(void(jm::dynamic_circular_buffer<int>{ cb.begin(), cb.end() }));

    jm::dynamic_circular_buffer<int> cb2(cb.begin(), cb.end());

    REQUIRE(std::equal(cb.begin(), cb.end(), cb2.begin()));
    REQUIRE(cb.size() == cb2.size());
  }

  jm::dynamic_circular_buffer<int> buf1{ 1, 2, 3, 4 };
  jm::dynamic_circular_buffer<int> buf2{ buf1.begin(), buf1.end() };

  REQUIRE(std::equal(buf1.begin(), buf1.end(), buf2.begin()));
  REQUIRE(buf1.size() == buf2.size());
}
#endif

TEST_CASE("n items construction")
{
    constexpr float               float_val = 2.f;
    jm::static_circular_buffer<float, 5> cb(4, float_val);
    for(auto item : cb)
        REQUIRE(item == float_val);

    REQUIRE(cb.size() == 4);
}

TEST_CASE("dynamic n items construction")
{
  constexpr float               float_val = 2.f;
  jm::dynamic_circular_buffer<float> cb(4, float_val);
  for (auto item : cb)
    REQUIRE(item == float_val);

  REQUIRE(cb.size() == 4);
}

TEST_CASE("clear empty full")
{
    {
        auto cb = gen_filled_cb(12);
        REQUIRE(cb.size() == 12);
        REQUIRE(!cb.empty());
        REQUIRE(!cb.full());
        cb.clear();
        REQUIRE(cb.empty());
        REQUIRE(!cb.full());
        REQUIRE(cb.size() == 0);
    }

    {
        auto cb = gen_filled_cb(16);
        REQUIRE(cb.size() == 16);
        REQUIRE(cb.size() == cb.max_size());

        REQUIRE(!cb.empty());
        REQUIRE(cb.full());
        cb.clear();
        REQUIRE(cb.empty());
        REQUIRE(!cb.full());
        REQUIRE(cb.size() == 0);
    }
}

TEST_CASE("dynamic clear empty full")
{
  {
    auto cb = dynamic_gen_filled_cb(12);
    REQUIRE(cb.size() == 12);
    REQUIRE(!cb.empty());
    REQUIRE(cb.full());
    cb.clear();
    REQUIRE(cb.empty());
    REQUIRE(!cb.full());
    REQUIRE(cb.size() == 0);
  }

  {
    auto cb = dynamic_gen_filled_cb(16);
    REQUIRE(cb.size() == 16);
    REQUIRE(cb.size() == cb.max_size());

    REQUIRE(!cb.empty());
    REQUIRE(cb.full());
    cb.clear();
    REQUIRE(cb.empty());
    REQUIRE(!cb.full());
    REQUIRE(cb.size() == 0);
  }
}

TEST_CASE("max_size")
{
    jm::static_circular_buffer<int, 5> cb1;
    REQUIRE(cb1.max_size() == 5);
}

TEST_CASE("dynamic max_size")
{
  jm::dynamic_circular_buffer<int> cb1(5);
  REQUIRE(cb1.max_size() == 5);
}

TEST_CASE("pop_back")
{
    auto cb = gen_filled_cb();

    for(int i = 15; i > 0; --i) {
        REQUIRE(cb.back() == i);
        cb.pop_back();
        REQUIRE(cb.size() == i);
    }

    REQUIRE(cb.front() == cb.back());
    cb.pop_back();
    cb.push_back(5);
    REQUIRE(cb.back() == 5);
    REQUIRE(cb.front() == 5);

    cb.push_back(6);
    REQUIRE(cb.back() == 6);
    REQUIRE(cb.front() == 5);
}

TEST_CASE("dynamic pop_back")
{
  auto cb = dynamic_gen_filled_cb();

  for (int i = 15; i > 0; --i) {
    REQUIRE(cb.back() == i);
    cb.pop_back();
    REQUIRE(cb.size() == i);
  }

  REQUIRE(cb.front() == cb.back());
  cb.pop_back();
  cb.push_back(5);
  REQUIRE(cb.back() == 5);
  REQUIRE(cb.front() == 5);

  cb.push_back(6);
  REQUIRE(cb.back() == 6);
  REQUIRE(cb.front() == 5);
}

TEST_CASE("pop_front")
{
    auto cb = gen_filled_cb();
    REQUIRE(cb.front() == 0);

    for(int i = 0; i < 15; ++i) {
        REQUIRE(cb.front() == i);
        cb.pop_front();
        REQUIRE(cb.size() == 15 - i);
    }

    REQUIRE(cb.front() == cb.back());
    cb.pop_front();
    cb.push_front(5);
    REQUIRE(cb.back() == 5);
    REQUIRE(cb.front() == 5);

    cb.push_front(6);
    REQUIRE(cb.back() == 5);
    REQUIRE(cb.front() == 6);
}
TEST_CASE("dynamic pop_front")
{
  auto cb = dynamic_gen_filled_cb();
  REQUIRE(cb.front() == 0);

  for (int i = 0; i < 15; ++i) {
    REQUIRE(cb.front() == i);
    cb.pop_front();
    REQUIRE(cb.size() == 15 - i);
  }

  REQUIRE(cb.front() == cb.back());
  cb.pop_front();
  cb.push_front(5);
  REQUIRE(cb.back() == 5);
  REQUIRE(cb.front() == 5);

  cb.push_front(6);
  REQUIRE(cb.back() == 5);
  REQUIRE(cb.front() == 6);
}

TEST_CASE("push_back")
{
    jm::static_circular_buffer<int, 16> cb;

    for(auto i : inc_vec) {
        cb.push_back(i);
        REQUIRE(cb.back() == i);
        REQUIRE(*--cb.end() == i);
        auto front = cb.front();
        for(auto v : cb)
            REQUIRE(v == front++);
    }

    REQUIRE(cb.size() == cb.max_size());
}
TEST_CASE("dynamic push_back")
{
  jm::dynamic_circular_buffer<int> cb;
  cb.reserve(inc_vec.size());

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
    jm::static_circular_buffer<int, 16> cb;

    for(auto i : inc_vec) {
        cb.push_front(i);
        REQUIRE(cb.front() == i);
        REQUIRE(*cb.begin() == i);
        for(auto v : cb)
            REQUIRE(v == i--);
    }

    REQUIRE(cb.size() == cb.max_size());
}

TEST_CASE("dynamic push_front")
{
  jm::dynamic_circular_buffer<int> cb;
  cb.reserve(inc_vec.size());
  for (auto i : inc_vec) {
    cb.push_front(i);
    REQUIRE(cb.front() == i);
    REQUIRE(*cb.begin() == i);
    for (auto v : cb)
      REQUIRE(v == i--);
  }

  REQUIRE(cb.size() == cb.max_size());
}

#ifndef JM_CIRCULAR_BUFFER_CXX_OLD
TEST_CASE("emplace_back")
{
    jm::static_circular_buffer<int, 16> cb;

    for(auto i : inc_vec) {
        cb.emplace_back(i);
        REQUIRE(cb.back() == i);
        REQUIRE(*--cb.end() == i);
        auto front = cb.front();
        for(auto v : cb)
            REQUIRE(v == front++);
    }

    REQUIRE(cb.size() == cb.max_size());
}

TEST_CASE("dynamic emplace_back")
{
  jm::dynamic_circular_buffer<int> cb;
  cb.reserve(inc_vec.size());

  for (auto i : inc_vec) {
    cb.emplace_back(i);
    REQUIRE(cb.back() == i);
    REQUIRE(*--cb.end() == i);
    auto front = cb.front();
    for (auto v : cb)
      REQUIRE(v == front++);
  }

  REQUIRE(cb.size() == cb.max_size());
}

TEST_CASE("emplace_front")
{
    jm::static_circular_buffer<int, 16> cb;

    for(auto i : inc_vec) {
        cb.emplace_front(i);
        REQUIRE(cb.front() == i);
        REQUIRE(*cb.begin() == i);
        for(auto v : cb)
            REQUIRE(v == i--);
    }

    REQUIRE(cb.size() == cb.max_size());
}
TEST_CASE("dynamic emplace_front")
{
  jm::dynamic_circular_buffer<int> cb;
  cb.reserve(inc_vec.size());

  for (auto i : inc_vec) {
    cb.emplace_front(i);
    REQUIRE(cb.front() == i);
    REQUIRE(*cb.begin() == i);
    for (auto v : cb)
      REQUIRE(v == i--);
  }

  REQUIRE(cb.size() == cb.max_size());
}
#endif

TEST_CASE("cb_iterator complies to Iterator")
{
    using cbt = jm::static_circular_buffer<int, 4>;
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
    { // MoveAssignable
      { auto t = cb.begin();
    t = cb.begin();
    static_assert(std::is_same<decltype(t = cb.begin()), cbt::iterator&>::value,
                  "t = cb.begin() doesnt return T&");

    REQUIRE((t = cb.end()) == t);
}
{
    auto t = cb.begin();
    auto v = cb.end();
    static_assert(std::is_same<decltype(t = v), cbt::iterator&>::value,
                  "t = cb.begin() doesnt return T&");

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
    using value_type        = cbt::iterator::value_type; // has all the typedefs
    using difference_type   = cbt::iterator::difference_type;
    using reference         = cbt::iterator::reference;
    using pointer           = cbt::iterator::pointer;
    using iterator_category = cbt::iterator::iterator_category;
}
*r; // r is dereferenceable
++r; // r is incrementable
static_assert(std::is_same<decltype(++r), decltype(r) &>::value,
              "++it doesnt return It&");
}

TEST_CASE("dynamic cb_iterator complies to Iterator")
{
  using cbt = jm::dynamic_circular_buffer<int>;
  cbt cb(2);
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
  { // MoveAssignable
    { auto t = cb.begin();
    t = cb.begin();
    static_assert(std::is_same<decltype(t = cb.begin()), cbt::iterator&>::value,
      "t = cb.begin() doesnt return T&");

    REQUIRE((t = cb.end()) == t);
    }
    {
      auto t = cb.begin();
      auto v = cb.end();
      static_assert(std::is_same<decltype(t = v), cbt::iterator&>::value,
        "t = cb.begin() doesnt return T&");

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
  static_assert(std::is_same<decltype(++r), decltype(r)&>::value,
    "++it doesnt return It&");
}


TEST_CASE("cb_iterator complies to InputIterator")
{
    using cbt = jm::static_circular_buffer<int, 4>;
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

    cbt::iterator       it = cb.begin();
    cbt::const_iterator non_c_tttt(it);
    cbt::const_iterator non_c_it = it;
    non_c_it                     = it;
}

TEST_CASE("dynamic cb_iterator complies to InputIterator")
{
  using cbt = jm::dynamic_circular_buffer<int>;
  cbt cb(2);
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

  cbt::iterator       it = cb.begin();
  cbt::const_iterator non_c_tttt(it);
  cbt::const_iterator non_c_it = it;
  non_c_it = it;
}

#include <Eigen/Geometry>
#include <Eigen/StdVector>

TEST_CASE("Eigen custom allocation Vector3f")
{
   
    jm::dynamic_circular_buffer<Eigen::Vector3f, Eigen::aligned_allocator< jm::detail::optional_storage<Eigen::Vector3f> >> buf1(8);
    jm::dynamic_circular_buffer<Eigen::Vector3f, Eigen::aligned_allocator< jm::detail::optional_storage<Eigen::Vector3f> >> buf2(8);

    for (int i = 0; i < 128; ++i) {
      buf1.push_back(Eigen::Vector3f::Random());
    }

    for (auto value : buf1) {
      buf2.push_back(value);
    }

    REQUIRE(buf1.size() == buf2.size());
    REQUIRE(std::equal(buf1.begin(), buf1.end(), buf2.begin()));
    *buf2.begin() = Eigen::Vector3f::Random();
    REQUIRE(!std::equal(buf1.begin(), buf1.end(), buf2.begin()));
}
//TEST_CASE("Eigen custom allocation Vector4f")
//{
//  jm::dynamic_circular_buffer<Eigen::Vector4f, Eigen::aligned_allocator<Eigen::Vector4f>> buf1(8);
//  jm::dynamic_circular_buffer<Eigen::Vector4f, Eigen::aligned_allocator<Eigen::Vector4f>> buf2(8);
//
//  for (int i = 0; i < 128; ++i) {
//    buf1.push_back(Eigen::Vector4f::Random());
//  }
//
//  for (auto value : buf1) {
//    buf2.push_back(value);
//  }
//
//  REQUIRE(buf1.size() == buf2.size());
//  REQUIRE(std::equal(buf1.begin(), buf1.end(), buf2.begin()));
//  *buf2.begin() = Eigen::Vector4f::Random();
//  REQUIRE(!std::equal(buf1.begin(), buf1.end(), buf2.begin()));
//}