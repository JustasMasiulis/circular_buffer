#ifndef JM_CIRCULAR_BUFFER_CONFIG_HPP
#define JM_CIRCULAR_BUFFER_CONFIG_HPP

#include <iterator>
#include <algorithm>
#include <stdexcept>

#include <array>
#include <vector>

#if !defined(JM_CIRCULAR_BUFFER_CXX_OLD)
#include <type_traits>
#include <initializer_list>
#endif // !defined(JM_CIRCULAR_BUFFER_CXX_OLD)


#ifndef JM_CIRCULAR_BUFFER_CXX_OLD
#define JM_CB_CONSTEXPR constexpr
#define JM_CB_NOEXCEPT noexcept
#define JM_CB_NULLPTR nullptr
#define JM_CB_ADDRESSOF(x) ::std::addressof(x)
#define JM_CB_IS_TRIVIALLY_DESTRUCTIBLE(type) \
    ::std::is_trivially_destructible<type>::value
#else
#define JM_CB_CONSTEXPR
#define JM_CB_NOEXCEPT
#define JM_CB_NULLPTR NULL
#define JM_CB_ADDRESSOF(x) &(x)
#define JM_CB_IS_TRIVIALLY_DESTRUCTIBLE(type) false
#endif

#ifdef JM_CIRCULAR_BUFFER_CXX14
#define JM_CB_CXX14_CONSTEXPR constexpr
#define JM_CB_CXX14_INIT_0 = 0
#else
#define JM_CB_CXX14_CONSTEXPR
#define JM_CB_CXX14_INIT_0
#endif

#if defined(__GNUC__)
#define JM_CB_LIKELY(x) __builtin_expect(x, 1)
#define JM_CB_UNLIKELY(x) __builtin_expect(x, 0)
#elif defined(__clang__) && !defined(__c2__) && defined(__has_builtin)
#if __has_builtin(__builtin_expect)
#define JM_CB_LIKELY(x) __builtin_expect(x, 1)
#define JM_CB_UNLIKELY(x) __builtin_expect(x, 0)
#endif
#endif


#ifndef JM_CB_LIKELY
#define JM_CB_LIKELY(expr) (expr)
#endif // !JM_CB_LIKELY


#ifndef JM_CB_UNLIKELY
#define JM_CB_UNLIKELY(expr) (expr)
#endif // !JM_CB_LIKELY


#if defined(JM_CIRCULAR_BUFFER_LIKELY_FULL) // optimization if you know if the buffer will
// likely be full or not
#define JM_CIRCULAR_BUFFER_FULLNESS_LIKEHOOD(expr) JM_CB_LIKELY(expr)
#elif defined(JM_CIRCULAR_BUFFER_UNLIKELY_FULL)
#define JM_CIRCULAR_BUFFER_FULLNESS_LIKEHOOD(expr) JM_CB_UNLIKELY(expr)
#else
#define JM_CIRCULAR_BUFFER_FULLNESS_LIKEHOOD(expr) expr
#endif

namespace jm::detail {
  template<class T>
  constexpr typename std::conditional<(!std::is_nothrow_move_assignable<T>::value&&
    std::is_copy_assignable<T>::value),
    const T&,
    T&&>::type
    move_if_noexcept_assign(T& arg) noexcept
  {
    return (std::move(arg));
  }

  template<class T, bool = JM_CB_IS_TRIVIALLY_DESTRUCTIBLE(T)>
  union optional_storage {
    struct empty_t {
    };

    empty_t _empty;
    T       _value;

    inline explicit JM_CB_CONSTEXPR optional_storage() JM_CB_NOEXCEPT : _empty()
    {}

    inline explicit JM_CB_CONSTEXPR
      optional_storage(const T& value) JM_CB_NOEXCEPT : _value(value)
    {}

    optional_storage(const optional_storage&) {}

    inline explicit constexpr optional_storage(T&& value)
      : _value(std::move(value))
    {}

    ~optional_storage() {}
  };

  template<class T>
  union optional_storage<T, true /* trivially destructible */> {
    struct empty_t {
    };

    empty_t _empty;
    T       _value;

    inline explicit JM_CB_CONSTEXPR optional_storage() JM_CB_NOEXCEPT : _empty()
    {}

    inline explicit JM_CB_CONSTEXPR
      optional_storage(const T& value) JM_CB_NOEXCEPT : _value(value)
    {}
    inline explicit constexpr optional_storage(T&& value)
      : _value(std::move(value))
    {}

    ~optional_storage() = default;
  };
}

#endif // JM_CIRCULAR_BUFFER_CONFIG_HPP