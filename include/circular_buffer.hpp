/*
* Copyright 2017 Justas Masiulis
*
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
*     http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*/

#ifndef JM_CIRCULAR_BUFFER_HPP
#define JM_CIRCULAR_BUFFER_HPP

#include <iterator>
#include <algorithm>
#include <stdexcept>

#if !defined(JM_CIRCULAR_BUFFER_CXX_OLD)
#include <initializer_list>
#endif // !defined(JM_CIRCULAR_BUFFER_CXX_OLD)


#ifndef JM_CIRCULAR_BUFFER_CXX_OLD
    #define BOOST_CONSTEXPR constexpr
    #define BOOST_NOEXCEPT noexcept
    #define BOOST_NULLPTR nullptr
    #define BOOST_ADDRESSOF(x) ::std::addressof(x)
#else
    #define BOOST_CONSTEXPR
    #define BOOST_NOEXCEPT
    #define BOOST_NULLPTR NULL
    #define BOOST_ADDRESSOF(x) &(x)
#endif

#ifdef JM_CIRCULAR_BUFFER_CXX14
    #define BOOST_CXX14_CONSTEXPR constexpr
#else
    #define BOOST_CXX14_CONSTEXPR
#endif


#if defined(__GNUC__)
    #define BOOST_LIKELY(x) __builtin_expect(x, 1)
    #define BOOST_UNLIKELY(x) __builtin_expect(x, 0)
#elif defined(__clang__) && !defined(__c2__) && defined(__has_builtin)
    #if __has_builtin(__builtin_expect)
        #define BOOST_LIKELY(x) __builtin_expect(x, 1)
        #define BOOST_UNLIKELY(x) __builtin_expect(x, 0)
    #endif
#endif


#ifndef BOOST_LIKELY
    #define BOOST_LIKELY(expr) (expr)
#endif // !BOOST_LIKELY


#ifndef BOOST_UNLIKELY
    #define BOOST_UNLIKELY(expr) (expr)
#endif // !BOOST_LIKELY


#if defined(JM_CIRCULAR_BUFFER_LIKELY_FULL) // optimization if you know if the buffer will likely be full or not
    #define JM_CIRCULAR_BUFFER_FULLNESS_LIKEHOOD(expr) BOOST_LIKELY(expr)
#elif defined(JM_CIRCULAR_BUFFER_UNLIKELY_FULL)
    #define JM_CIRCULAR_BUFFER_FULLNESS_LIKEHOOD(expr) BOOST_UNLIKELY(expr)
#else
    #define JM_CIRCULAR_BUFFER_FULLNESS_LIKEHOOD(expr) expr
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


        struct empty_t{};


        template<typename T>
        union optional_storage 
        {
            empty_t _empty;
            T       _value;

            BOOST_CONSTEXPR optional_storage() BOOST_NOEXCEPT
                : _empty()
            {}

            BOOST_CONSTEXPR optional_storage(const T& value) BOOST_NOEXCEPT
                : _value(value)
            {}

#if !defined(BOOST_NO_CXX11_RVALUE_REFERENCES) && !defined(JM_CIRCULAR_BUFFER_CXX_OLD)

            BOOST_CONSTEXPR optional_storage(T&& value) BOOST_NOEXCEPT
                : _value(std::move(value))
            {}

#endif

#if !defined(BOOST_NO_CXX11_DEFAULTED_FUNCTIONS) && !defined(JM_CIRCULAR_BUFFER_CXX_OLD)

            ~optional_storage() = default;

#endif
        };

#if defined(JM_CIRCULAR_BUFFER_CXX_OLD) || defined(JM_CIRCULAR_BUFFER_SPEED_OVER_CONSTEXPR)

        template<typename T, std::size_t N>
        struct cb_storage
        {
            T _buffer[N];

            const T& at(std::size_t idx) const
            {
                return _buffer[idx];
            }

            T& at(std::size_t idx)
            {
                return _buffer[idx];
            }
        };

#elif defined(JM_CIRCULAR_BUFFER_CXX14)

        template<typename T, std::size_t N>
        struct cb_storage
        {
            optional_storage<T> _buffer[N];

            inline constexpr const T& at(std::size_t idx) const noexcept
            {
                return _buffer[idx]._value;
            }

            inline constexpr T& at(std::size_t idx) noexcept
            {
                return _buffer[idx]._value;
            }
        };

#else

        template<typename T, std::size_t N>
        struct cb_storage
        {
            optional_storage<T[N]> _buffer;

            inline constexpr const T& at(std::size_t idx) const noexcept
            {
                return _buffer._value[idx];
            }

            inline constexpr T& at(std::size_t idx) noexcept
            {
                return _buffer._value[idx];
            }
        };

#endif


    }


    template<typename S, typename TC, std::size_t N>
    class circular_buffer_iterator
    {
    public:
        typedef std::bidirectional_iterator_tag iterator_category;
        typedef TC                              value_type;
        typedef std::ptrdiff_t                  difference_type;
        typedef value_type*                     pointer;
        typedef value_type&                     reference;

    private:
        S*          _buf;
        std::size_t _pos;
        std::size_t _left_in_forward;

        typedef detail::cb_index_wrapper<std::size_t, N> wrapper_t;

    public:
        explicit BOOST_CONSTEXPR circular_buffer_iterator() BOOST_NOEXCEPT
            : _buf(BOOST_NULLPTR)
            , _pos(0)
            , _left_in_forward(0)
        {}

        explicit BOOST_CONSTEXPR circular_buffer_iterator(S* buf, std::size_t pos, std::size_t left_in_forward) BOOST_NOEXCEPT
            : _buf(buf)
            , _pos(pos)
            , _left_in_forward(left_in_forward)
        {}

        template<typename TSnc, typename Tnc>
        explicit BOOST_CONSTEXPR circular_buffer_iterator(const circular_buffer_iterator<TSnc, Tnc, N>& lhs) BOOST_NOEXCEPT
            : _buf(lhs._buf)
            , _pos(lhs._pos)
            , _left_in_forward(lhs._left_in_forward)
        {}

        BOOST_CONSTEXPR reference operator*() const BOOST_NOEXCEPT
        {
            return (_buf + _pos)->_value;
        }

        BOOST_CONSTEXPR pointer operator->() const BOOST_NOEXCEPT
        {
            return BOOST_ADDRESSOF((_buf + _pos)->_value);
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

        template<typename Tx, typename Ty>
        BOOST_CONSTEXPR bool operator==(const circular_buffer_iterator<Tx, Ty, N>& lhs) const BOOST_NOEXCEPT
        {
            return lhs._pos == _pos && lhs._left_in_forward == _left_in_forward && lhs._buf == _buf;
        }

        template<typename Tx, typename Ty>
        BOOST_CONSTEXPR bool operator!=(const circular_buffer_iterator<Tx, Ty, N>& lhs) const BOOST_NOEXCEPT
        {
            return !(operator==(lhs));
        }
    };
 

    template<typename T, std::size_t N>
    class circular_buffer
    {
    public: 
        typedef T                                                        value_type;
        typedef std::size_t                                              size_type;
        typedef std::ptrdiff_t                                           difference_type;
        typedef T&                                                       reference;
        typedef const T&                                                 const_reference;
        typedef T*                                                       pointer;
        typedef const T*                                                 const_pointer;
        typedef circular_buffer_iterator<detail::optional_storage<T>, T, N>       iterator;
        typedef circular_buffer_iterator<const detail::optional_storage<T>, const T, N> const_iterator;
        typedef std::reverse_iterator<iterator>                          reverse_iterator;
        typedef std::reverse_iterator<const_iterator>                    const_reverse_iterator;
        
    private:
        typedef detail::cb_index_wrapper<size_type, N>                   wrapper_t;
        typedef detail::optional_storage<T>                                    storage_type;

        size_type    _head;
        size_type    _tail;
        size_type    _size;
        storage_type _buffer[N];

        inline BOOST_CXX14_CONSTEXPR void destroy(size_type idx) BOOST_NOEXCEPT
        {
            _buffer[idx]._value.~T();
        }

        BOOST_CXX14_CONSTEXPR inline void copy_range(const storage_type* buffer
                                                     , size_type first, size_type last)
        {
            for (size_type i = first; i < last; ++i)
                _buffer[i]._value = (buffer + i)->_value;
        }

        BOOST_CXX14_CONSTEXPR void copy_buffer(const storage_type* buffer)
        {
            if (JM_CIRCULAR_BUFFER_FULLNESS_LIKEHOOD(_size == N))
                copy_range(buffer, 0, N);
            else if (_head > _tail)
                copy_range(buffer, _tail, _head + 1);
            else
                copy_range(buffer, _head, _tail + 1);
        }


#if !defined(JM_CIRCULAR_BUFFER_CXX_OLD)

        BOOST_CXX14_CONSTEXPR inline void move_range(storage_type* buffer
                                                     , size_type first, size_type last)
        {
            for (size_type i = first; i < last; ++i)
                _buffer[i]._value = std::move((buffer + i)->_value);
        }

        BOOST_CXX14_CONSTEXPR void move_buffer(storage_type* buffer)
        {
            if (JM_CIRCULAR_BUFFER_FULLNESS_LIKEHOOD(_size == N))
                move_range(buffer, 0, N);
            else if (_head > _tail)
                move_range(buffer, _tail, _head + 1);
            else
                move_range(buffer, _head, _tail + 1);
        }

#endif // !defined(JM_CIRCULAR_BUFFER_CXX_OLD)
    
    public:
        BOOST_CONSTEXPR explicit circular_buffer()
            : _head(1)
            , _tail(0)
            , _size(0)
            , _buffer()
        {}

#if defined(JM_CIRCULAR_BUFFER_CXX14)

        constexpr circular_buffer(size_type count, const T& value)
            : _head(0)
            , _tail(count - 1)
            , _size(count)
            , _buffer()
        {
            if (BOOST_UNLIKELY(_size > N))
                throw std::out_of_range("circular_buffer<T, N>(size_type count, const T&) count exceeded N");

            if (BOOST_LIKELY(_size != 0))
                for (size_type i = 0; i < count; ++i)
                    _buffer[i] = storage_type(value);
            else
                _head = 1;
        }

#else

        explicit circular_buffer(size_type count, const T& value = T())
            : _head(0)
            , _tail(count - 1)
            , _size(count)
            , _buffer()
        {
            if (BOOST_UNLIKELY(_size > N))
                throw std::out_of_range("circular_buffer<T, N>(size_type count, const T&) count exceeded N");

            if (BOOST_LIKELY(_size != 0))
                for (size_type i = 0; i < count; ++i)
                    _buffer[i] = storage_type(value);
            else
                _head = 1;
        }

#endif

        template<typename InputIt>
        BOOST_CXX14_CONSTEXPR circular_buffer(InputIt first, InputIt last)
            : _head(0)
            , _tail(0)
            , _size(0)
            , _buffer()
        {
            if (first != last) {
                for (; first != last; ++first, ++_size) {
                    if (BOOST_UNLIKELY(_size >= N))
                        throw std::out_of_range("circular_buffer<T, N>(InputIt first, InputIt last) distance exceeded N");

                    _buffer[_size] = storage_type(*first);
                }

                _tail = _size - 1;
            }
            else
                _head = 1;
        }

#if !defined(JM_CIRCULAR_BUFFER_CXX_OLD)

        BOOST_CXX14_CONSTEXPR circular_buffer(std::initializer_list<T> init)
            : _head(0)
            , _tail(init.size() - 1)
            , _size(init.size())
            , _buffer()
        {
            if (BOOST_UNLIKELY(_size > N))
                throw std::out_of_range("circular_buffer<T, N>(std::initializer_list<T> init) init.size() > N");

            if (BOOST_UNLIKELY(_size == 0))
                _head = 1;

            storage_type* buf_ptr = _buffer;
            for (auto it = init.begin(), end = init.end(); it != end; ++it, ++buf_ptr)
                *buf_ptr = std::move(storage_type(std::move(*it)));
        }

#endif // !defined(JM_CIRCULAR_BUFFER_CXX_OLD)

        BOOST_CXX14_CONSTEXPR circular_buffer(const circular_buffer& other)
            : _head(other._head)
            , _tail(other._tail)
            , _size(other._size)
            , _buffer()
        {
            copy_buffer(other._buffer);
        }

        BOOST_CXX14_CONSTEXPR circular_buffer& operator=(const circular_buffer& other)
        {
            _head = other._head;
            _tail = other._tail;
            _size = other._size;

            copy_buffer(other._buffer);

            return *this;
        }

#if !defined(JM_CIRCULAR_BUFFER_CXX_OLD)

        BOOST_CXX14_CONSTEXPR circular_buffer(circular_buffer&& other)
            : _head(other._head)
            , _tail(other._tail)
            , _size(other._size)
            , _buffer()
        {
            move_buffer(other._buffer);
        }

        BOOST_CXX14_CONSTEXPR circular_buffer& operator=(circular_buffer&& other)
        {
            _head = other._head;
            _tail = other._tail;
            _size = other._size;

            move_buffer(other._buffer);

            return *this;
        }

#endif // !defined(JM_CIRCULAR_BUFFER_CXX_OLD)

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
            return _buffer[_head]._value;
        }
        
        BOOST_CONSTEXPR const_reference front() const BOOST_NOEXCEPT
        {
            return _buffer[_head]._value;
        }
        
        BOOST_CXX14_CONSTEXPR reference back() BOOST_NOEXCEPT
        {
            return _buffer[_tail]._value;
        }
        
        BOOST_CONSTEXPR const_reference back() const BOOST_NOEXCEPT
        {
            return _buffer[_tail]._value;
        }

        BOOST_CXX14_CONSTEXPR pointer data() BOOST_NOEXCEPT
        {
            return BOOST_ADDRESSOF(_buffer[0]._value);
        }

        BOOST_CONSTEXPR const_pointer data() const BOOST_NOEXCEPT
        {
            return BOOST_ADDRESSOF(_buffer[0]._value);
        }
        
    /// modifiers
        BOOST_CXX14_CONSTEXPR void push_back(const value_type& value)
        {
            size_type new_tail;
            if(JM_CIRCULAR_BUFFER_FULLNESS_LIKEHOOD(_size == N)) {
                new_tail = _head;
                _head = wrapper_t::increment(_head);
                --_size;
            }
            else
                new_tail = wrapper_t::increment(_tail);

            _buffer[new_tail]._value = value; 
            _tail = new_tail;
            ++_size;
        }

        BOOST_CXX14_CONSTEXPR void push_front(const value_type& value)
        {
            size_type new_head;
            if (JM_CIRCULAR_BUFFER_FULLNESS_LIKEHOOD(_size == N)) {
                new_head = _tail;
                _tail = wrapper_t::decrement(_tail);
                --_size;
            }
            else
                new_head = wrapper_t::decrement(_head);

            _buffer[new_head]._value = value;
            _head = new_head;
            ++_size;
        }

#if !defined(JM_CIRCULAR_BUFFER_CXX_OLD)

        BOOST_CXX14_CONSTEXPR void push_back(value_type&& value)
        {
            size_type new_tail;
            if (JM_CIRCULAR_BUFFER_FULLNESS_LIKEHOOD(_size == N)) {
                new_tail = _head;
                _head = wrapper_t::increment(_head);
                --_size;
            }
            else
                new_tail = wrapper_t::increment(_tail);

            _buffer[new_tail]._value = std::move(value);
            _tail = new_tail;
            ++_size;
        }

        BOOST_CXX14_CONSTEXPR void push_front(value_type&& value)
        {
            size_type new_head;
            if (JM_CIRCULAR_BUFFER_FULLNESS_LIKEHOOD(_size == N)) {
                new_head = _tail;
                _tail = wrapper_t::decrement(_tail);
                --_size;
            }
            else
                new_head = wrapper_t::decrement(_head);

            _buffer[new_head]._value = std::move(value);
            _head = new_head;
            ++_size;
        }


        template<typename... Args>
        void emplace_back(Args&&... args)
        {
            size_type new_tail; 
            if (JM_CIRCULAR_BUFFER_FULLNESS_LIKEHOOD(_size == N)) {
                new_tail = _head;
                _head = wrapper_t::increment(_head);
                --_size;
                destroy(new_tail);
            }
            else
                new_tail = wrapper_t::increment(_tail);

            new (BOOST_ADDRESSOF(_buffer[new_tail]._value)) value_type(std::forward<Args>(args)...);
            _tail = new_tail;
            ++_size;
        }

        template<typename... Args>
        void emplace_front(Args&&... args)
        {
            size_type new_head;
            if (JM_CIRCULAR_BUFFER_FULLNESS_LIKEHOOD(_size == N)) {
                new_head = _tail;
                _tail = wrapper_t::decrement(_tail);
                --_size;
                destroy(new_head);
            }
            else
                new_head = wrapper_t::decrement(_head);

            new (BOOST_ADDRESSOF(_buffer[new_head]._value)) value_type(std::forward<Args>(args)...);
            _head = new_head;
            ++_size;
        }

#endif// !defined(JM_CIRCULAR_BUFFER_CXX_OLD)
        
        BOOST_CXX14_CONSTEXPR void pop_back() BOOST_NOEXCEPT
        {
            size_type old_tail = _tail;
            _tail = wrapper_t::decrement(_tail);
            --_size;
            destroy(old_tail);
        }
        
        BOOST_CXX14_CONSTEXPR void pop_front() BOOST_NOEXCEPT
        {
            size_type old_head = _head;
            _head = wrapper_t::decrement(_head);
            --_size;
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
