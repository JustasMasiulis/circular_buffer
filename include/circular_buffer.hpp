#ifndef JM_CIRCULAR_BUFFER_HPP
#define JM_CIRCULAR_BUFFER_HPP

#include <iterator>
#include <algorithm>
#include <initializer_list>

#include <boost/config.hpp>


#if defined(CIRCULAR_BUFFER_LIKELY_FULL)
    #define JM_CIRCULAR_BUFFER_FULLNESS_LIKEHOOD(expr) BOOST_LIKELY(expr)
#elif defined(CIRCULAR_BUFFER_UNLIKELY_FULL)
    #define JM_CIRCULAR_BUFFER_FULLNESS_LIKEHOOD(expr) BOOST_UNLIKELY(expr)
#else
    #define JM_CIRCULAR_BUFFER_FULLNESS_LIKEHOOD(expr) expr
#endif

#ifdef BOOST_NO_NULLPTR
    #define JM_NULLPTR NULL
#else
    #define JM_NULLPTR nullptr
#endif

namespace jm
{

    template<typename T, std::size_t N>
    class circular_buffer_iterator
        : public std::iterator<std::bidirectional_iterator_tag
                              ,  T
                              ,  ptrdiff_t
                              ,  T*
                              ,  T&>
    {
        pointer     _buf;
        std::size_t _pos;
        std::size_t _left_in_forward;

        BOOST_CONSTEXPR void increment() BOOST_NOEXCEPT
        {
            _pos = (_pos + 1) % N;
        }

        BOOST_CONSTEXPR void decrement() BOOST_NOEXCEPT
        {
            _pos = (_pos + N - 1) % N;
        }

    public:
        explicit BOOST_CONSTEXPR circular_buffer_iterator() BOOST_NOEXCEPT
            : _buf(JM_NULLPTR)
            , _pos(0)
            , _left_in_forward(0)
        {}

        explicit BOOST_CONSTEXPR circular_buffer_iterator(pointer buf, std::size_t pos, std::size_t left_in_forward) BOOST_NOEXCEPT
            : _buf(buf)
            , _pos(pos)
            , _left_in_forward(left_in_forward)
        {}

        template<typename Tnc>
        explicit BOOST_CONSTEXPR circular_buffer_iterator(const circular_buffer_iterator<Tnc, N>& lhs)
            : _buf(lhs._buf)
            , _pos(lhs._pos)
            , _left_in_forward(lhs._left_in_forward)
        {}

        BOOST_CONSTEXPR reference operator*() const BOOST_NOEXCEPT
        {
            return *(_buf + _pos);
        }

        BOOST_CONSTEXPR pointer operator->() const BOOST_NOEXCEPT
        {
            return _buf + _pos;
        }

        BOOST_CXX14_CONSTEXPR circular_buffer_iterator& operator++() BOOST_NOEXCEPT
        {
            increment();
            --_left_in_forward;
            return *this;
        }

        BOOST_CXX14_CONSTEXPR circular_buffer_iterator& operator--() BOOST_NOEXCEPT
        {
            decrement();
            ++_left_in_forward;
            return *this;
        }

        BOOST_CXX14_CONSTEXPR circular_buffer_iterator operator++(int) BOOST_NOEXCEPT
        {
            circular_buffer_iterator temp = *this;
            increment();
            --_left_in_forward;
            return temp;
        }

        BOOST_CXX14_CONSTEXPR circular_buffer_iterator operator--(int) BOOST_NOEXCEPT
        {
            auto temp = *this;
            decrement();
            ++_left_in_forward;
            return temp;
        }

        template<typename Tx>
        BOOST_CONSTEXPR bool operator==(const circular_buffer_iterator<Tx, N>& lhs) const BOOST_NOEXCEPT
        {
            return lhs._pos == _pos && lhs._left_in_forward == _left_in_forward && lhs._buf == _buf;
        }

        template<typename Tx>
        BOOST_CONSTEXPR bool operator!=(const circular_buffer_iterator<Tx, N>& lhs) const BOOST_NOEXCEPT
        {
            return !(operator==(lhs));
        }
    };
 

    template<typename T, std::size_t N>
    class circular_buffer
    {
    public: 
        typedef T                                     value_type; 
        typedef std::size_t                           size_type;
        typedef std::ptrdiff_t                        difference_type;
        typedef T&                                    reference;
        typedef const T&                              const_reference;
        typedef T*                                    pointer;
        typedef const T*                              const_pointer;
        typedef circular_buffer_iterator<T, N>        iterator;
        typedef circular_buffer_iterator<const T, N>  const_iterator;
        typedef std::reverse_iterator<iterator>       reverse_iterator;
        typedef std::reverse_iterator<const_iterator> const_reverse_iterator;
        
    private:
        size_type  _head;
        size_type  _tail;
        size_type  _size;
        value_type _buffer[N];
        
        BOOST_CONSTEXPR size_type increment(size_type idx) const BOOST_NOEXCEPT
        {
            return (idx + 1) % N;
        }
        
        BOOST_CONSTEXPR size_type decrement(size_type idx) const BOOST_NOEXCEPT
        {
            return (idx + N - 1) % (N);
        }

        BOOST_CONSTEXPR void destroy(size_type idx) BOOST_NOEXCEPT
        {
            _buffer[idx].~T();
        }

        template<typename... Args>
        inline void construct(size_type idx, Args&&... args)
        {
            new (&_buffer[idx]) T(std::forward<Args>(args)...);
        }

        BOOST_CXX11_CONSTEXPR void copy_buffer(const T* buffer)
        {
            if (_size == N)
                std::copy(buffer, buffer + N, ::std::addressof(_buffer[0]));
            else if (_head > _tail)
                std::copy(buffer + _tail, buffer + _head + 1, ::std::addressof(_buffer[0]) + _tail);
            else
                std::copy(buffer + _head, buffer + _tail + 1, ::std::addressof(_buffer[0]) + _head);
        }

        BOOST_CXX14_CONSTEXPR void move_buffer(T* buffer)
        {
            if (_size == N)
                std::move(buffer, buffer + N, std::addressof(_buffer[0]));
            else if (_head > _tail)
                std::move(buffer + _tail, buffer + _head + 1, ::std::addressof(_buffer[0]) + _tail);
            else
                std::move(buffer + _head, buffer + _tail + 1, ::std::addressof(_buffer[0]) + _head);
        }
    
    public:
        explicit circular_buffer()
            : _head(1)
            , _tail(0)
            , _size(0)
        {}

        explicit circular_buffer(size_type count, const T& value)
            : _head(0)
            , _tail(count - 1)
            , _size(count)
        {
            if (count > N)
                throw std::range_error("circular_buffer<T, N>(size_type count, const T&) count exceeded N");

            std::fill(_buffer, _buffer + count, value);
        }

        template<typename InputIt>
        circular_buffer(InputIt first, InputIt last)
            : _head(0)
            , _tail(std::distance(first, last) - 1)
            , _size(_tail + 1)
        {
            if (_size > N)
                throw std::range_error("circular_buffer<T, N>(InputIt first, InputIt last) distance exceeded N");

            std::copy(first, last, ::std::addressof(_buffer[0]));
        }

        circular_buffer(const circular_buffer& other)
            : _head(other._head)
            , _tail(other._tail)
            , _size(other._size)
        {
            copy_buffer(::std::addressof(other._buffer[0]));
        }

        BOOST_CXX14_CONSTEXPR circular_buffer(circular_buffer&& other)
            : _head(other._head)
            , _tail(other._tail)
            , _size(other._size)
        {
            move_buffer(::std::addressof(other._buffer[0]));
        }

        BOOST_CXX14_CONSTEXPR circular_buffer(std::initializer_list<T> init)
            : _size(init.size())
            , _head(0)
            , _tail(_size - 1)
        {
            if (_size > N)
                throw std::length_error("circular_buffer<T, N>(std::initializer_list<T> init) init.size() > N");

            std::move(init.begin(), init.end(), ::std::addressof(_buffer[0]));
        }

        BOOST_CXX14_CONSTEXPR circular_buffer& operator=(const circular_buffer& other)
        {
            _head = other._head;
            _tail = other._tail;
            _size = other._size;

            copy_buffer(::std::addressof(other._buffer[0]));

            return *this;
        }

        BOOST_CXX14_CONSTEXPR circular_buffer& operator=(circular_buffer&& other)
        {
            _head = other._head;
            _tail = other._tail;
            _size = other._size;

            move_buffer(::std::addressof(other._buffer[0]));

            return *this;
        }

    /// capacity
        constexpr bool empty() const BOOST_NOEXCEPT
        {
            return _size == 0;
        }

        constexpr bool full() const BOOST_NOEXCEPT
        {
            return _size == N;
        }
        
        constexpr size_type size() const BOOST_NOEXCEPT
        {
            return _size;
        }
        
        constexpr size_type max_size() const BOOST_NOEXCEPT
        {
            return N; 
        }
       
    /// element access
        constexpr reference front() BOOST_NOEXCEPT
        {
            return _buffer[_head];
        }
        
        constexpr const_reference front() const BOOST_NOEXCEPT
        {
            return _buffer[_head];
        }
        
        constexpr reference back() BOOST_NOEXCEPT
        {
            return _buffer[_tail];
        }
        
        constexpr const_reference back() const BOOST_NOEXCEPT
        {
            return _buffer[_tail];
        }

        constexpr T* data() BOOST_NOEXCEPT
        {
            return _buffer;
        }

        constexpr const T* data() const BOOST_NOEXCEPT
        {
            return &_buffer;
        }
        
    /// modifiers
        BOOST_CXX14_CONSTEXPR void push_back(const value_type& value)
        {
            auto new_tail = increment(_tail);
            if(_size == N) {
                _head = increment(_head);
                --_size;
            }
            _buffer[new_tail] = value; 
            _tail = new_tail;
            ++_size;
        }
        
        BOOST_CXX14_CONSTEXPR void push_back(value_type&& value)
        {
            auto new_tail = increment(_tail);
            if(_size == N) {
                _head = increment(_head);
                --_size;
            }
            _buffer[new_tail] = value; 
            _tail = new_tail;
            ++_size;
        }
        
        BOOST_CXX14_CONSTEXPR void push_front(const value_type& value)
        {
            auto new_head = decrement(_head);
            if(_size == N) {
                _tail = decrement(_tail);
                --_size;
            }
            _buffer[new_head] = value; 
            _head = new_head;
            ++_size;
        }
        
        BOOST_CXX14_CONSTEXPR void push_front(value_type&& value)
        {
            auto new_head = decrement(_head);
            if(_size == N) {
                _tail = decrement(_tail);
                --_size;
            }
            _buffer[new_head] = std::move(value); 
            _head = new_head;
            ++_size;
        }
        
        template<typename... Args>
        void emplace_back(Args&&... args)
        {
            auto new_tail = increment(_tail);
            if (_size == N) {
                _head = increment(_head);
                --_size;
                destroy(new_tail);
            }
            construct(new_tail, std::forward<Args>(args)...);
            _tail = new_tail;
            ++_size;
        }

        template<typename... Args>
        void emplace_front(Args&&... args)
        {
            auto new_head = decrement(_head);
            if (_size == N) {
                _tail = decrement(_tail);
                --_size;
                destroy(new_head)
            }
            construct(new_head, std::forward<Args>(args)...);
            _head = new_head;
            ++_size;
        }
        
        BOOST_CXX14_CONSTEXPR void pop_back() BOOST_NOEXCEPT
        {
            auto old_tail = _tail;
            _tail = decrement(_tail);
            _buffer[old_tail].~value_type();
        }
        
        BOOST_CXX14_CONSTEXPR void pop_front() BOOST_NOEXCEPT
        {
            auto old_head = _head;
            _head = decrement(_head);
            _buffer[old_head].~value_type();
        }

        BOOST_CXX14_CONSTEXPR void clear() BOOST_NOEXCEPT
        {
            while (_size != 0)
                pop_back();

            _head = 1;
            _tail = 0;
        }

    /// iterators
        iterator begin() BOOST_NOEXCEPT
        {
            if (_size == 0)
                return end();
            return iterator(_buffer, _head, _size);
        }

        const_iterator begin() const BOOST_NOEXCEPT
        {
            if (_size == 0)
                return end();
            return const_iterator(_buffer, _head, _size);
        }

        const_iterator cbegin() const BOOST_NOEXCEPT
        {
            if (_size == 0)
                return cend();
            return const_iterator(_buffer, _head, _size);
        }

        reverse_iterator rbegin() BOOST_NOEXCEPT
        {
            if (_size == 0)
                return rend();
            return reverse_iterator(iterator{ _buffer, _head, _size });
        }

        const_reverse_iterator  rbegin() const BOOST_NOEXCEPT
        {
            if (_size == 0)
                return rend();
            return const_reverse_iterator(const_iterator{ _buffer, _head, _size });
        }

        const_reverse_iterator  crbegin() const BOOST_NOEXCEPT
        {
            if (_size == 0)
                return crend();
            return const_reverse_iterator(const_iterator{ _buffer, _head, _size});
        }

        iterator end() BOOST_NOEXCEPT
        {
            return iterator(_buffer, increment(_tail), 0);
        }

        const_iterator end() const BOOST_NOEXCEPT
        {
            return const_iterator(_buffer, increment(_tail), 0);
        }

        const_iterator cend() const BOOST_NOEXCEPT
        {
            return const_iterator(_buffer, increment(_tail), 0);
        }

        reverse_iterator rend() BOOST_NOEXCEPT
        {
            return reverse_iterator(iterator{ _buffer, increment(_tail), 0 });
        }

        const_reverse_iterator rend() const BOOST_NOEXCEPT
        {
            return const_reverse_iterator(const_iterator{ _buffer, increment(_tail), 0 });
        }

        const_reverse_iterator crend() const BOOST_NOEXCEPT
        {
            return const_reverse_iterator(const_iterator{ _buffer, increment(_tail), 0 });
        }
    };

}

#endif JM_CIRCULAR_BUFFER_HPP
