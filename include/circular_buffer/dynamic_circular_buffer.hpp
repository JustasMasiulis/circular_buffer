#ifndef JM_DYNAMIC_CIRCULAR_BUFFER_HPP
#define JM_DYNAMIC_CIRCULAR_BUFFER_HPP

#include <circular_buffer/detail/dynamic_iterator.hpp>

namespace jm {

  template<typename T, class Allocator = std::allocator<detail::optional_storage<T>>>
  class dynamic_circular_buffer {
  public:
    typedef std::vector<detail::optional_storage<T>, Allocator>    container;
    typedef T                                                      value_type;
    typedef std::size_t                                            size_type;
    typedef std::ptrdiff_t                                         difference_type;
    typedef T& reference;
    typedef const T& const_reference;
    typedef T* pointer;
    typedef const T* const_pointer;
    typedef detail::cb_iterator<detail::optional_storage<T>, T, 0> iterator;
    typedef detail::cb_iterator<const detail::optional_storage<T>, const T, 0>
      const_iterator;
    typedef std::reverse_iterator<iterator>       reverse_iterator;
    typedef std::reverse_iterator<const_iterator> const_reverse_iterator;

  private:
    typedef detail::cb_index_wrapper<size_type, 0> wrapper_t;
    typedef T            storage_type;

    size_type    _head;
    size_type    _tail;
    size_type    _size;
    container    _buffer;

    inline void destroy(size_type idx) JM_CB_NOEXCEPT { _buffer[idx]._value.~T(); }

    inline void copy_buffer(const dynamic_circular_buffer& other)
    {
      const_iterator       first = other.cbegin();
      const const_iterator last = other.cend();

      for (; first != last; ++first)
        push_back(*first);
    }

    inline void move_buffer(dynamic_circular_buffer&& other)
    {
      reserve(other.max_size());

      iterator       first = other.begin();
      const iterator last = other.end();

      for (; first != last; ++first)
        emplace_back(std::move(*first));
    }

  public:
    JM_CB_CONSTEXPR explicit dynamic_circular_buffer()
      : _head(1), _tail(0), _size(0), _buffer()
    {  }

    explicit
      dynamic_circular_buffer(size_type count)
      : _head(0), _tail(count - 1), _size(count), _buffer(count)
    {
    }

    explicit
      dynamic_circular_buffer(size_type count, const T& value)
      : _head(0), _tail(count - 1), _size(count), _buffer(count)
    {

      if (JM_CB_UNLIKELY(_size > _buffer.size()))
        throw std::out_of_range(
          "dynamic_circular_buffer<T, N>(size_type count, const T&) count exceeded N");

      if (JM_CB_LIKELY(_size != 0))
        for (size_type i = 0; i < count; ++i)
          new(JM_CB_ADDRESSOF(_buffer[i])) T(value);
      else
        _head = 1;
    }

    template<typename InputIt>
    dynamic_circular_buffer(InputIt first, InputIt last)
      : _head(0), _tail(0), _size(0), _buffer(std::distance(first, last))
    {
      if (first != last) {
        for (; first != last; ++first, ++_size) {
          if (JM_CB_UNLIKELY(_size >= _buffer.size()))
            throw std::out_of_range(
              "dynamic_circular_buffer<T, N>(InputIt first, InputIt last) distance exceeded N");

          new(JM_CB_ADDRESSOF(_buffer[_size])) T(*first);
        }

        _tail = _size - 1;
      }
      else
        _head = 1;
    }

    dynamic_circular_buffer(std::initializer_list<T> init)
      : _head(0), _tail(init.size() - 1), _size(init.size()), _buffer(init.size())
    {
      if (JM_CB_UNLIKELY(_size > _buffer.size()))
        throw std::out_of_range(
          "circular_buffer<T, N>(std::initializer_list<T> init) init.size() > N");


      if (JM_CB_UNLIKELY(_size == 0))
        _head = 1;

      auto buf_ptr = _buffer.begin();
      for (auto it = init.begin(), end = init.end(); it < end; ++it, ++buf_ptr) {
        new(JM_CB_ADDRESSOF(*buf_ptr)) T(*it);
      }

    }

    dynamic_circular_buffer(const dynamic_circular_buffer& other)
      : _head(1), _tail(0), _size(0), _buffer(other.max_size())
    {
      copy_buffer(other);
    }

    dynamic_circular_buffer& operator=(const dynamic_circular_buffer& other)
    {
      if (other.max_size() != max_size()) throw std::runtime_error("other.max_size != max_size()");
      clear();
      copy_buffer(other);
      return *this;
    }

    dynamic_circular_buffer(dynamic_circular_buffer&& other) JM_CB_NOEXCEPT : _head(1), _tail(0), _size(0), _buffer()
    {
      move_buffer(std::move(other));
    }

    dynamic_circular_buffer& operator=(dynamic_circular_buffer&& other) JM_CB_NOEXCEPT
    {
      clear();
      move_buffer(std::move(other));
      return *this;
    }

    ~dynamic_circular_buffer() { clear(); }

    /// capacity
    JM_CB_CONSTEXPR void reserve(size_type new_cap) {
      if (JM_CB_UNLIKELY(!_buffer.empty())) throw std::runtime_error("reserve called once");

      _buffer = container(new_cap);
    }

    JM_CB_CONSTEXPR void resize(size_type new_size) {
      if (JM_CB_UNLIKELY(new_size > _buffer.size())) throw std::runtime_error("new_cap > max_size()");

      const auto current_size = size();
      difference_type count = static_cast<difference_type>(new_size - current_size);
      const bool isexpend = count >= 0;

      if (JM_CB_LIKELY(isexpend)) {
        while (count--) {
          push_back({});
        }
      }
      else {
        while (count++) {
          pop_back();
        }
      }
    }
    JM_CB_CONSTEXPR size_type capacity() const JM_CB_NOEXCEPT {
      return _buffer.size();
    }

    JM_CB_CONSTEXPR bool empty() const JM_CB_NOEXCEPT { return _size == 0; }

    JM_CB_CONSTEXPR bool full() const JM_CB_NOEXCEPT { return _size == _buffer.size(); }

    JM_CB_CONSTEXPR size_type size() const JM_CB_NOEXCEPT { return _size; }

    JM_CB_CONSTEXPR size_type max_size() const JM_CB_NOEXCEPT { return _buffer.size(); }

    /// element access
    JM_CB_CXX14_CONSTEXPR reference front() JM_CB_NOEXCEPT
    {
      return _buffer[_head]._value;
    }

    JM_CB_CONSTEXPR const_reference front() const JM_CB_NOEXCEPT
    {
      return _buffer[_head]._value;
    }

    JM_CB_CXX14_CONSTEXPR reference back() JM_CB_NOEXCEPT
    {
      return _buffer[_tail]._value;
    }

    JM_CB_CONSTEXPR const_reference back() const JM_CB_NOEXCEPT
    {
      return _buffer[_tail]._value;
    }

    JM_CB_CXX14_CONSTEXPR pointer data() JM_CB_NOEXCEPT
    {
      return JM_CB_ADDRESSOF(_buffer[0]);
    }

    JM_CB_CONSTEXPR const_pointer data() const JM_CB_NOEXCEPT
    {
      return JM_CB_ADDRESSOF(_buffer[0]);
    }

    /// modifiers
    void push_back(const value_type& value)
    {
      size_type new_tail;
      if (JM_CIRCULAR_BUFFER_FULLNESS_LIKEHOOD(_size == _buffer.size())) {
        new_tail = _head;
        _head = wrapper_t::increment(_head, _buffer.size());
        --_size;
        _buffer[new_tail]._value = value;
      }
      else {
        new_tail = wrapper_t::increment(_tail, _buffer.size());
        new(JM_CB_ADDRESSOF(_buffer[new_tail])) T(value);
      }

      _tail = new_tail;
      ++_size;
    }

    void push_front(const value_type& value)
    {
      size_type new_head = 0;
      if (JM_CIRCULAR_BUFFER_FULLNESS_LIKEHOOD(_size == _buffer.size())) {
        new_head = _tail;
        _tail = wrapper_t::decrement(_tail, _buffer.size());
        --_size;
        _buffer[new_head]._value = value;
      }
      else {
        new_head = wrapper_t::decrement(_head, _buffer.size());
        new(JM_CB_ADDRESSOF(_buffer[new_head])) T(value);
      }

      _head = new_head;
      ++_size;
    }

    void push_back(value_type&& value)
    {
      size_type new_tail;
      if (JM_CIRCULAR_BUFFER_FULLNESS_LIKEHOOD(_size == _buffer.size())) {
        new_tail = _head;
        _head = wrapper_t::increment(_head, _buffer.size());
        --_size;
        _buffer[new_tail]._value = detail::move_if_noexcept_assign(value);
      }
      else {
        new_tail = wrapper_t::increment(_tail, _buffer.size());
        new(JM_CB_ADDRESSOF(_buffer[new_tail]))
          T(std::move_if_noexcept(value));
      }

      _tail = new_tail;
      ++_size;
    }

    void push_front(value_type&& value)
    {
      size_type new_head = 0;
      if (JM_CIRCULAR_BUFFER_FULLNESS_LIKEHOOD(_size == _buffer.size())) {
        new_head = _tail;
        _tail = wrapper_t::decrement(_tail, _buffer.size());
        --_size;
        _buffer[new_head]._value = detail::move_if_noexcept_assign(value);
      }
      else {
        new_head = wrapper_t::decrement(_head, _buffer.size());
        new(JM_CB_ADDRESSOF(_buffer[new_head]))
          T(std::move_if_noexcept(value));
      }

      _head = new_head;
      ++_size;
    }

    template<typename... Args>
    void emplace_back(Args&&... args)
    {
      size_type new_tail;
      if (JM_CIRCULAR_BUFFER_FULLNESS_LIKEHOOD(_size == _buffer.size())) {
        new_tail = _head;
        _head = wrapper_t::increment(_head, _buffer.size());
        --_size;
        destroy(new_tail);
      }
      else
        new_tail = wrapper_t::increment(_tail, _buffer.size());

      new(JM_CB_ADDRESSOF(_buffer[new_tail]))
        value_type(std::forward<Args>(args)...);
      _tail = new_tail;
      ++_size;
    }

    template<typename... Args>
    void emplace_front(Args&&... args)
    {
      size_type new_head;
      if (JM_CIRCULAR_BUFFER_FULLNESS_LIKEHOOD(_size == _buffer.size())) {
        new_head = _tail;
        _tail = wrapper_t::decrement(_tail, _buffer.size());
        --_size;
        destroy(new_head);
      }
      else
        new_head = wrapper_t::decrement(_head, _buffer.size());

      new(JM_CB_ADDRESSOF(_buffer[new_head]))
        value_type(std::forward<Args>(args)...);
      _head = new_head;
      ++_size;
    }

    JM_CB_CXX14_CONSTEXPR void pop_back() JM_CB_NOEXCEPT
    {
      size_type old_tail = _tail;
      --_size;
      _tail = wrapper_t::decrement(_tail, _buffer.size());
      destroy(old_tail);
    }

    JM_CB_CXX14_CONSTEXPR void pop_front() JM_CB_NOEXCEPT
    {
      size_type old_head = _head;
      --_size;
      _head = wrapper_t::increment(_head, _buffer.size());
      destroy(old_head);
    }

    JM_CB_CXX14_CONSTEXPR void clear() JM_CB_NOEXCEPT
    {
      while (_size != 0)
        pop_back();

      _head = 1;
      _tail = 0;
    }

    /// iterators
    JM_CB_CXX14_CONSTEXPR iterator begin() JM_CB_NOEXCEPT
    {
      if (_size == 0)
        return end();
      return iterator(_buffer.data(), _head, _size, _buffer.size());
    }

    JM_CB_CXX14_CONSTEXPR const_iterator begin() const JM_CB_NOEXCEPT
    {
      if (_size == 0)
        return end();
      return const_iterator(_buffer.data(), _head, _size, _buffer.size());
    }

    JM_CB_CXX14_CONSTEXPR const_iterator cbegin() const JM_CB_NOEXCEPT
    {
      if (_size == 0)
        return cend();
      return const_iterator(_buffer.data(), _head, _size, _buffer.size());
    }

    JM_CB_CXX14_CONSTEXPR reverse_iterator rbegin() JM_CB_NOEXCEPT
    {
      if (_size == 0)
        return rend();
      return reverse_iterator(iterator(_buffer.data(), _head, _size, _buffer.size()));
    }

    JM_CB_CXX14_CONSTEXPR const_reverse_iterator rbegin() const JM_CB_NOEXCEPT
    {
      if (_size == 0)
        return rend();
      return const_reverse_iterator(const_iterator(_buffer.data(), _head, _size, _buffer.size()));
    }

    JM_CB_CXX14_CONSTEXPR const_reverse_iterator crbegin() const JM_CB_NOEXCEPT
    {
      if (_size == 0)
        return crend();
      return const_reverse_iterator(const_iterator(_buffer.data(), _head, _size, _buffer.size()));
    }

    JM_CB_CXX14_CONSTEXPR iterator end() JM_CB_NOEXCEPT
    {
      return iterator(_buffer.data(), wrapper_t::increment(_tail, _buffer.size()), 0, _buffer.size());
    }

    JM_CB_CXX14_CONSTEXPR const_iterator end() const JM_CB_NOEXCEPT
    {
      return const_iterator(_buffer.data(), wrapper_t::increment(_tail, _buffer.size()), 0, _buffer.size());
    }

    JM_CB_CXX14_CONSTEXPR const_iterator cend() const JM_CB_NOEXCEPT
    {
      return const_iterator(_buffer.data(), wrapper_t::increment(_tail, _buffer.size()), 0, _buffer.size());
    }

    JM_CB_CXX14_CONSTEXPR reverse_iterator rend() JM_CB_NOEXCEPT
    {
      return reverse_iterator(iterator(_buffer.data(), wrapper_t::increment(_tail, _buffer.size()), 0, _buffer.size()));
    }

    JM_CB_CXX14_CONSTEXPR const_reverse_iterator rend() const JM_CB_NOEXCEPT
    {
      return const_reverse_iterator(
        const_iterator(_buffer.data(), wrapper_t::increment(_tail, _buffer.size()), 0, _buffer.size()));
    }

    JM_CB_CXX14_CONSTEXPR const_reverse_iterator crend() const JM_CB_NOEXCEPT
    {
      return const_reverse_iterator(
        const_iterator(_buffer.data(), wrapper_t::increment(_tail, _buffer.size()), 0, _buffer.size()));
    }
  };
} // namespace jm

#endif // include guard
