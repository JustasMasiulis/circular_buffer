#ifndef JM_CIRCULAR_BUFFER_HPP
#define JM_CIRCULAR_BUFFER_HPP

#ifndef JM_CIRCULAR_BUFFER_NO_BOOST
#include <boost/config.hpp>
#endif // !JM_CIRCULAR_BUFFER_NO_BOOST

#include <iterator>
#include <algorithm>
#include <stdexcept>

#if !defined(BOOST_NO_INITIALIZER_LISTS) && !defined(JM_CIRCULAR_BUFFER_CXX_OLD)
#include <initializer_list>
#endif // !BOOST_NO_INITIALIZER_LISTS

#ifdef JM_CIRCULAR_BUFFER_NO_BOOST // defaults to c++11 features
    #define BOOST_LIKELY(expr) expr
    #define BOOST_UNLIKELY(expr) expr

    #ifndef JM_CIRCULAR_BUFFER_CXX_OLD
        #define BOOST_CONSTEXPR constexpr
        #define BOOST_NOEXCEPT noexcept
    #else
        #define BOOST_CONSTEXPR
        #define BOOST_NOEXCEPT
    #endif

    #ifdef JM_CIRCULAR_BUFFER_CXX14
        #define BOOST_CXX14_CONSTEXPR constexpr
    #else
        #define BOOST_CXX14_CONSTEXPR
    #endif
#endif

#if defined(CIRCULAR_BUFFER_LIKELY_FULL)
    #define JM_CIRCULAR_BUFFER_FULLNESS_LIKEHOOD(expr) BOOST_LIKELY(expr)
#elif defined(CIRCULAR_BUFFER_UNLIKELY_FULL)
    #define JM_CIRCULAR_BUFFER_FULLNESS_LIKEHOOD(expr) BOOST_UNLIKELY(expr)
#else
    #define JM_CIRCULAR_BUFFER_FULLNESS_LIKEHOOD(expr) expr
#endif

#if defined(BOOST_NO_NULLPTR) || defined(JM_CIRCULAR_BUFFER_CXX_OLD)
    #define BOOST_NULLPTR NULL
#else
    #define BOOST_NULLPTR nullptr
#endif

#if defined(BOOST_NO_CXX11_ADDRESSOF) || defined(JM_CIRCULAR_BUFFER_CXX_OLD)
    #define BOOST_ADDRESSOF(x) &(x)
#else
    #define BOOST_ADDRESSOF(x) ::std::addressof(x)
#endif

namespace jm
{

    namespace detail
    {

        template<typename size_type, size_type N>
        struct cb_index_wrapper
        {
            inline static BOOST_CONSTEXPR size_type increment(size_type value) BOOST_NOEXCEPT
            {
                return (value + 1) % N;
            }

            inline static BOOST_CONSTEXPR size_type decrement(size_type value) BOOST_NOEXCEPT
            {
                return (value + N - 1) % N;
            }
        };

    }


    template<typename T, std::size_t N>
    class circular_buffer_iterator
    {
    public:
        typedef std::bidirectional_iterator_tag iterator_category;
        typedef T                               value_type;
        typedef std::ptrdiff_t                  difference_type;
        typedef T*                              pointer;
        typedef T&                              reference;

    private:
        pointer     _buf;
        std::size_t _pos;
        std::size_t _left_in_forward;

        typedef detail::cb_index_wrapper<std::size_t, N> wrapper_t;

    public:
        explicit BOOST_CONSTEXPR circular_buffer_iterator() BOOST_NOEXCEPT
            : _buf(BOOST_NULLPTR)
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
            _pos = wrapper_t::increment(_pos);
            --_left_in_forward;
            return *this;
        }

        BOOST_CXX14_CONSTEXPR circular_buffer_iterator& operator--() BOOST_NOEXCEPT
        {
            _pos = wrapper_t::decrement(_pos);
            ++_left_in_forward;
            return *this;
        }

        BOOST_CXX14_CONSTEXPR circular_buffer_iterator operator++(int) BOOST_NOEXCEPT
        {
            circular_buffer_iterator temp = *this;
            _pos = wrapper_t::increment(_pos);
            --_left_in_forward;
            return temp;
        }

        BOOST_CXX14_CONSTEXPR circular_buffer_iterator operator--(int) BOOST_NOEXCEPT
        {
            circular_buffer_iterator temp = *this;
            _pos = wrapper_t::decrement(_pos);
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

        typedef detail::cb_index_wrapper<size_type, N> wrapper_t;

        inline BOOST_CXX14_CONSTEXPR void destroy(size_type idx) BOOST_NOEXCEPT
        {
            _buffer[idx].~T();
        }

        BOOST_CXX14_CONSTEXPR void copy_buffer(const T* buffer)
        {
            if (_size == N)
                std::copy(buffer, buffer + N, BOOST_ADDRESSOF(_buffer[0]));
            else if (_head > _tail)
                std::copy(buffer + _tail, buffer + _head + 1, BOOST_ADDRESSOF(_buffer[0]) + _tail);
            else
                std::copy(buffer + _head, buffer + _tail + 1, BOOST_ADDRESSOF(_buffer[0]) + _head);
        }


#if !defined(BOOST_NO_RVALUE_REFERENCES) && !defined(JM_CIRCULAR_BUFFER_CXX_OLD)

        BOOST_CXX14_CONSTEXPR void move_buffer(T* buffer)
        {
            if (_size == N)
                std::move(buffer, buffer + N, BOOST_ADDRESSOF(_buffer[0]));
            else if (_head > _tail)
                std::move(buffer + _tail, buffer + _head + 1, BOOST_ADDRESSOF(_buffer[0]) + _tail);
            else
                std::move(buffer + _head, buffer + _tail + 1, BOOST_ADDRESSOF(_buffer[0]) + _head);
        }

#endif // !BOOST_NO_RVALUE_REFERENCES
    
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
                throw std::out_of_range("circular_buffer<T, N>(size_type count, const T&) count exceeded N");

            std::fill(_buffer, _buffer + count, value);
        }

        template<typename InputIt>
        circular_buffer(InputIt first, InputIt last)
            : _head(0)
            , _tail(std::distance(first, last) - 1)
            , _size(_tail + 1)
        {
            if (_size > N)
                throw std::out_of_range("circular_buffer<T, N>(InputIt first, InputIt last) distance exceeded N");

            std::copy(first, last, BOOST_ADDRESSOF(_buffer[0]));
        }

#if !defined(BOOST_NO_INITIALIZER_LISTS) && !defined(JM_CIRCULAR_BUFFER_CXX_OLD)

        circular_buffer(std::initializer_list<T> init)
            : _size(init.size())
            , _head(0)
            , _tail(_size - 1)
        {
            if (_size > N)
                throw std::out_of_range("circular_buffer<T, N>(std::initializer_list<T> init) init.size() > N");

            std::move(init.begin(), init.end(), BOOST_ADDRESSOF(_buffer[0]));
        }

#endif // !BOOST_NO_INITIALIZER_LISTS

        circular_buffer(const circular_buffer& other)
            : _head(other._head)
            , _tail(other._tail)
            , _size(other._size)
        {
            copy_buffer(BOOST_ADDRESSOF(other._buffer[0]));
        }

        BOOST_CXX14_CONSTEXPR circular_buffer& operator=(const circular_buffer& other)
        {
            _head = other._head;
            _tail = other._tail;
            _size = other._size;

            copy_buffer(BOOST_ADDRESSOF(other._buffer[0]));

            return *this;
        }

#if !defined(BOOST_NO_RVALUE_REFERENCES) && !defined(JM_CIRCULAR_BUFFER_CXX_OLD)

        circular_buffer(circular_buffer&& other)
            : _head(other._head)
            , _tail(other._tail)
            , _size(other._size)
        {
            move_buffer(BOOST_ADDRESSOF(other._buffer[0]));
        }

        BOOST_CXX14_CONSTEXPR circular_buffer& operator=(circular_buffer&& other)
        {
            _head = other._head;
            _tail = other._tail;
            _size = other._size;

            move_buffer(BOOST_ADDRESSOF(other._buffer[0]));

            return *this;
        }

#endif

    /// capacity
        BOOST_CONSTEXPR bool empty() const BOOST_NOEXCEPT
        {
            return _size == 0;
        }

        BOOST_CONSTEXPR bool full() const BOOST_NOEXCEPT
        {
            return _size == N;
        }
        
        BOOST_CONSTEXPR size_type size() const BOOST_NOEXCEPT
        {
            return _size;
        }
        
        BOOST_CONSTEXPR size_type max_size() const BOOST_NOEXCEPT
        {
            return N; 
        }
       
    /// element access
        BOOST_CXX14_CONSTEXPR reference front() BOOST_NOEXCEPT
        {
            return _buffer[_head];
        }
        
        BOOST_CONSTEXPR const_reference front() const BOOST_NOEXCEPT
        {
            return _buffer[_head];
        }
        
        BOOST_CXX14_CONSTEXPR reference back() BOOST_NOEXCEPT
        {
            return _buffer[_tail];
        }
        
        BOOST_CONSTEXPR const_reference back() const BOOST_NOEXCEPT
        {
            return _buffer[_tail];
        }

        BOOST_CXX14_CONSTEXPR pointer data() BOOST_NOEXCEPT
        {
            return _buffer;
        }

        BOOST_CONSTEXPR const_pointer data() const BOOST_NOEXCEPT
        {
            return _buffer;
        }
        
    /// modifiers
        BOOST_CXX14_CONSTEXPR void push_back(const value_type& value)
        {
            size_type new_tail = wrapper_t::increment(_tail);
            if(JM_CIRCULAR_BUFFER_FULLNESS_LIKEHOOD(_size == N)) {
                _head = wrapper_t::increment(_head);
                --_size;
            }
            _buffer[new_tail] = value; 
            _tail = new_tail;
            ++_size;
        }

        BOOST_CXX14_CONSTEXPR void push_front(const value_type& value)
        {
            size_type new_head = wrapper_t::decrement(_head);
            if (JM_CIRCULAR_BUFFER_FULLNESS_LIKEHOOD(_size == N)) {
                _tail = wrapper_t::decrement(_tail);
                --_size;
            }
            _buffer[new_head] = value;
            _head = new_head;
            ++_size;
        }

#if !defined(BOOST_NO_RVALUE_REFERENCES) && !defined(JM_CIRCULAR_BUFFER_CXX_OLD)

        BOOST_CXX14_CONSTEXPR void push_back(value_type&& value)
        {
            size_type new_tail = wrapper_t::increment(_tail);
            if (JM_CIRCULAR_BUFFER_FULLNESS_LIKEHOOD(_size == N)) {
                _head = wrapper_t::increment(_head);
                --_size;
            }
            _buffer[new_tail] = value;
            _tail = new_tail;
            ++_size;
        }

        BOOST_CXX14_CONSTEXPR void push_front(value_type&& value)
        {
            size_type new_head = wrapper_t::decrement(_head);
            if (JM_CIRCULAR_BUFFER_FULLNESS_LIKEHOOD(_size == N)) {
                _tail = wrapper_t::decrement(_tail);
                --_size;
            }
            _buffer[new_head] = std::move(value);
            _head = new_head;
            ++_size;
        }

#endif // !BOOST_NO_RVALUE_REFERENCES

#if !defined(BOOST_NO_CXX11_VARIADIC_TEMPLATES) && !defined(JM_CIRCULAR_BUFFER_CXX_OLD)

        template<typename... Args>
        void emplace_back(Args&&... args)
        {
            size_type new_tail = wrapper_t::increment(_tail);
            if (JM_CIRCULAR_BUFFER_FULLNESS_LIKEHOOD(_size == N)) {
                _head = wrapper_t::increment(_head);
                --_size;
                destroy(new_tail);
            }
            new (&_buffer[new_tail]) value_type(std::forward<Args>(args)...);
            _tail = new_tail;
            ++_size;
        }

        template<typename... Args>
        void emplace_front(Args&&... args)
        {
            size_type new_head = wrapper_t::decrement(_head);
            if (JM_CIRCULAR_BUFFER_FULLNESS_LIKEHOOD(_size == N)) {
                _tail = wrapper_t::decrement(_tail);
                --_size;
                destroy(new_head);
            }
            new (&_buffer[new_head]) value_type(std::forward<Args>(args)...);
            _head = new_head;
            ++_size;
        }

#endif
        
        BOOST_CXX14_CONSTEXPR void pop_back() BOOST_NOEXCEPT
        {
            size_type old_tail = _tail;
            _tail = wrapper_t::decrement(_tail);
            destroy(old_tail);
        }
        
        BOOST_CXX14_CONSTEXPR void pop_front() BOOST_NOEXCEPT
        {
            size_type old_head = _head;
            _head = wrapper_t::decrement(_head);
            destroy(old_head);
        }

        BOOST_CXX14_CONSTEXPR void clear() BOOST_NOEXCEPT
        {
            while (_size != 0)
                pop_back();

            _head = 1;
            _tail = 0;
        }

    /// iterators
        BOOST_CXX14_CONSTEXPR iterator begin() BOOST_NOEXCEPT
        {
            if (_size == 0)
                return end();
            return iterator(_buffer, _head, _size);
        }

        BOOST_CXX14_CONSTEXPR const_iterator begin() const BOOST_NOEXCEPT
        {
            if (_size == 0)
                return end();
            return const_iterator(_buffer, _head, _size);
        }

        BOOST_CXX14_CONSTEXPR const_iterator cbegin() const BOOST_NOEXCEPT
        {
            if (_size == 0)
                return cend();
            return const_iterator(_buffer, _head, _size);
        }

        BOOST_CXX14_CONSTEXPR reverse_iterator rbegin() BOOST_NOEXCEPT
        {
            if (_size == 0)
                return rend();
            return reverse_iterator(iterator(_buffer, _head, _size));
        }

        BOOST_CXX14_CONSTEXPR const_reverse_iterator  rbegin() const BOOST_NOEXCEPT
        {
            if (_size == 0)
                return rend();
            return const_reverse_iterator(const_iterator(_buffer, _head, _size));
        }

        BOOST_CXX14_CONSTEXPR const_reverse_iterator  crbegin() const BOOST_NOEXCEPT
        {
            if (_size == 0)
                return crend();
            return const_reverse_iterator(const_iterator(_buffer, _head, _size));
        }

        BOOST_CXX14_CONSTEXPR iterator end() BOOST_NOEXCEPT
        {
            return iterator(_buffer, wrapper_t::increment(_tail), 0);
        }

        BOOST_CXX14_CONSTEXPR const_iterator end() const BOOST_NOEXCEPT
        {
            return const_iterator(_buffer, wrapper_t::increment(_tail), 0);
        }

        BOOST_CXX14_CONSTEXPR const_iterator cend() const BOOST_NOEXCEPT
        {
            return const_iterator(_buffer, wrapper_t::increment(_tail), 0);
        }

        BOOST_CXX14_CONSTEXPR reverse_iterator rend() BOOST_NOEXCEPT
        {
            return reverse_iterator(iterator( _buffer, wrapper_t::increment(_tail), 0 ));
        }

        BOOST_CXX14_CONSTEXPR const_reverse_iterator rend() const BOOST_NOEXCEPT
        {
            return const_reverse_iterator(const_iterator(_buffer, wrapper_t::increment(_tail), 0));
        }

        BOOST_CXX14_CONSTEXPR const_reverse_iterator crend() const BOOST_NOEXCEPT
        {
            return const_reverse_iterator(const_iterator(_buffer, wrapper_t::increment(_tail), 0));
        }
    };

}

#endif // !JM_CIRCULAR_BUFFER_HPP
