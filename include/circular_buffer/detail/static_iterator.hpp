#ifndef JM_ITERATOR_STATIC_CIRCULAR_BUFFER_HPP
#define JM_ITERATOR_STATIC_CIRCULAR_BUFFER_HPP

#include <circular_buffer/config.hpp>

namespace jm {

  namespace detail {

    template<class size_type, size_type N>
    struct cb_index_wrapper {
      inline static JM_CB_CONSTEXPR size_type increment(size_type value)
        JM_CB_NOEXCEPT
      {
        return (value + 1) % N;
      }

      inline static JM_CB_CONSTEXPR size_type decrement(size_type value)
        JM_CB_NOEXCEPT
      {
        return (value + N - 1) % N;
      }
    };

    template<class S, class TC, std::size_t N>
    class cb_iterator {
      template<class, class, std::size_t>
      friend class cb_iterator;

      S* _buf;
      std::size_t _pos;
      std::size_t _left_in_forward;

      typedef detail::cb_index_wrapper<std::size_t, N> wrapper_t;

    public:
      typedef std::bidirectional_iterator_tag iterator_category;
      typedef TC                              value_type;
      typedef std::ptrdiff_t                  difference_type;
      typedef value_type* pointer;
      typedef value_type& reference;

      explicit JM_CB_CONSTEXPR cb_iterator() JM_CB_NOEXCEPT : _buf(JM_CB_NULLPTR),
        _pos(0),
        _left_in_forward(0)
      {}

      explicit JM_CB_CONSTEXPR
        cb_iterator(S* buf,
          std::size_t pos,
          std::size_t left_in_forward) JM_CB_NOEXCEPT
        : _buf(buf),
        _pos(pos),
        _left_in_forward(left_in_forward)
      {}

      template<class TSnc, class Tnc>
      JM_CB_CONSTEXPR
        cb_iterator(const cb_iterator<TSnc, Tnc, N>& other) JM_CB_NOEXCEPT
        : _buf(other._buf),
        _pos(other._pos),
        _left_in_forward(other._left_in_forward)
      {}

      template<class TSnc, class Tnc>
      JM_CB_CXX14_CONSTEXPR cb_iterator&
        operator=(const cb_iterator<TSnc, Tnc, N>& other) JM_CB_NOEXCEPT
      {
        _buf = other._buf;
        _pos = other._pos;
        _left_in_forward = other._left_in_forward;
        return *this;
      };


      JM_CB_CONSTEXPR reference operator*() const JM_CB_NOEXCEPT
      {
        return (_buf + _pos)->_value;
      }

      JM_CB_CONSTEXPR pointer operator->() const JM_CB_NOEXCEPT
      {
        return JM_CB_ADDRESSOF((_buf + _pos)->_value);
      }

      JM_CB_CXX14_CONSTEXPR cb_iterator& operator++() JM_CB_NOEXCEPT
      {
        _pos = wrapper_t::increment(_pos);
        --_left_in_forward;
        return *this;
      }

      JM_CB_CXX14_CONSTEXPR cb_iterator& operator--() JM_CB_NOEXCEPT
      {
        _pos = wrapper_t::decrement(_pos);
        ++_left_in_forward;
        return *this;
      }

      JM_CB_CXX14_CONSTEXPR cb_iterator operator++(int)JM_CB_NOEXCEPT
      {
        cb_iterator temp = *this;
        _pos = wrapper_t::increment(_pos);
        --_left_in_forward;
        return temp;
      }

      JM_CB_CXX14_CONSTEXPR cb_iterator operator--(int)JM_CB_NOEXCEPT
      {
        cb_iterator temp = *this;
        _pos = wrapper_t::decrement(_pos);
        ++_left_in_forward;
        return temp;
      }

      template<class Tx, class Ty>
      JM_CB_CONSTEXPR bool
        operator==(const cb_iterator<Tx, Ty, N>& lhs) const JM_CB_NOEXCEPT
      {
        return lhs._left_in_forward == _left_in_forward && lhs._pos == _pos &&
          lhs._buf == _buf;
      }

      template<typename Tx, typename Ty>
      JM_CB_CONSTEXPR bool
        operator!=(const cb_iterator<Tx, Ty, N>& lhs) const JM_CB_NOEXCEPT
      {
        return !(operator==(lhs));
      }
    };
  } // namespace detail
}

#endif