
#include <circular_buffer/config.hpp>

namespace jm {

  namespace detail {
    template<>
    struct cb_index_wrapper<std::size_t, 0> {
      inline static JM_CB_CONSTEXPR std::size_t increment(std::size_t value, std::size_t max_value)
        JM_CB_NOEXCEPT
      {
        return (value + 1) % max_value;
      }

      inline static JM_CB_CONSTEXPR std::size_t decrement(std::size_t value, std::size_t max_value)
        JM_CB_NOEXCEPT
      {
        return (value + max_value - 1) % max_value;
      }
    };

    // special case when we need dynamic buffer and we can't specify size buffer at compile time
    // so we make N = 0 and use specialize cb_index_wrapper template 
    template<class S, class TC>
    class cb_iterator<S, TC, 0> {

      template<class, class, std::size_t>
      friend class cb_iterator;

      S* _buf;
      std::size_t _pos;
      std::size_t _left_in_forward;
      std::size_t _max_size;

      typedef detail::cb_index_wrapper<std::size_t, 0> wrapper_t;

    public:
      typedef std::bidirectional_iterator_tag iterator_category;
      typedef TC                              value_type;
      typedef std::ptrdiff_t                  difference_type;
      typedef value_type* pointer;
      typedef value_type& reference;

      explicit JM_CB_CONSTEXPR cb_iterator() JM_CB_NOEXCEPT : _buf(JM_CB_NULLPTR),
        _pos(0),
        _left_in_forward(0),
        _max_size(0)
      {}

      explicit JM_CB_CONSTEXPR
        cb_iterator(S* buf,
          std::size_t pos,
          std::size_t left_in_forward,
          std::size_t max_size) JM_CB_NOEXCEPT
        : _buf(buf),
        _pos(pos),
        _left_in_forward(left_in_forward),
        _max_size(max_size)
      {}

      template<class TSnc, class Tnc>
      JM_CB_CONSTEXPR
        cb_iterator(const cb_iterator<TSnc, Tnc, 0>& other) JM_CB_NOEXCEPT
        : _buf(other._buf),
        _pos(other._pos),
        _left_in_forward(other._left_in_forward),
        _max_size(other._max_size)
      {}

      template<class TSnc, class Tnc>
      JM_CB_CXX14_CONSTEXPR cb_iterator&
        operator=(const cb_iterator<TSnc, Tnc, 0>& other) JM_CB_NOEXCEPT
      {
        _buf = other._buf;
        _pos = other._pos;
        _left_in_forward = other._left_in_forward;
        _max_size = other._max_size;
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
        _pos = wrapper_t::increment(_pos, _max_size);
        --_left_in_forward;
        return *this;
      }

      JM_CB_CXX14_CONSTEXPR cb_iterator& operator--() JM_CB_NOEXCEPT
      {
        _pos = wrapper_t::decrement(_pos, _max_size);
        ++_left_in_forward;
        return *this;
      }

      JM_CB_CXX14_CONSTEXPR cb_iterator operator++(int)JM_CB_NOEXCEPT
      {
        cb_iterator temp = *this;
        _pos = wrapper_t::increment(_pos, _max_size);
        --_left_in_forward;
        return temp;
      }

      JM_CB_CXX14_CONSTEXPR cb_iterator operator--(int)JM_CB_NOEXCEPT
      {
        cb_iterator temp = *this;
        _pos = wrapper_t::decrement(_pos, _max_size);
        ++_left_in_forward;
        return temp;
      }

      template<class Tx, class Ty>
      JM_CB_CONSTEXPR bool
        operator==(const cb_iterator<Tx, Ty, 0>& lhs) const JM_CB_NOEXCEPT
      {
        return lhs._left_in_forward == _left_in_forward && lhs._pos == _pos &&
          lhs._buf == _buf && lhs._max_size == _max_size;
      }

      template<typename Tx, typename Ty>
      JM_CB_CONSTEXPR bool
        operator!=(const cb_iterator<Tx, Ty, 0>& lhs) const JM_CB_NOEXCEPT
      {
        return !(operator==(lhs));
      }
    };
  } // namespace detail
}