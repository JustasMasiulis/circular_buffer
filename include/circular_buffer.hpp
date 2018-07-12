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

#if !defined(JM_CIRCULAR_BUFFER_CXX_OLD)

        template<class T>
        constexpr typename std::conditional<(!std::is_nothrow_move_assignable<T>::value &&
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

#else

        template<class T>
        union optional_storage {
            alignas(T) char _value[sizeof(T)];
            T _value;

            inline explicit JM_CB_CONSTEXPR optional_storage() JM_CB_NOEXCEPT : _empty()
            {}

            inline explicit JM_CB_CONSTEXPR
            optional_storage(const T& value) JM_CB_NOEXCEPT : _value(value)
            {}

            ~optional_storage() {}

            inline explicit constexpr optional_storage(T&& value)
                : _value(std::move(value))
            {}
        };

#endif

        template<class S, class TC, std::size_t N>
        class cb_iterator {
            template<class, class, std::size_t>
            friend class cb_iterator;

            S*          _buf;
            std::size_t _pos;
            std::size_t _left_in_forward;

            typedef detail::cb_index_wrapper<std::size_t, N> wrapper_t;

        public:
            typedef std::bidirectional_iterator_tag iterator_category;
            typedef TC                              value_type;
            typedef std::ptrdiff_t                  difference_type;
            typedef value_type*                     pointer;
            typedef value_type&                     reference;

            explicit JM_CB_CONSTEXPR cb_iterator() JM_CB_NOEXCEPT : _buf(JM_CB_NULLPTR),
                                                                    _pos(0),
                                                                    _left_in_forward(0)
            {}

            explicit JM_CB_CONSTEXPR
            cb_iterator(S*          buf,
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
                _buf             = other._buf;
                _pos             = other._pos;
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
                _pos             = wrapper_t::increment(_pos);
                --_left_in_forward;
                return temp;
            }

            JM_CB_CXX14_CONSTEXPR cb_iterator operator--(int)JM_CB_NOEXCEPT
            {
                cb_iterator temp = *this;
                _pos             = wrapper_t::decrement(_pos);
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


    template<typename T, std::size_t N>
    class circular_buffer {
    public:
        typedef T                                                      value_type;
        typedef std::size_t                                            size_type;
        typedef std::ptrdiff_t                                         difference_type;
        typedef T&                                                     reference;
        typedef const T&                                               const_reference;
        typedef T*                                                     pointer;
        typedef const T*                                               const_pointer;
        typedef detail::cb_iterator<detail::optional_storage<T>, T, N> iterator;
        typedef detail::cb_iterator<const detail::optional_storage<T>, const T, N>
                                                      const_iterator;
        typedef std::reverse_iterator<iterator>       reverse_iterator;
        typedef std::reverse_iterator<const_iterator> const_reverse_iterator;

    private:
        typedef detail::cb_index_wrapper<size_type, N> wrapper_t;
        typedef detail::optional_storage<T>            storage_type;

        size_type    _head;
        size_type    _tail;
        size_type    _size;
        storage_type _buffer[N];

        inline void destroy(size_type idx) JM_CB_NOEXCEPT { _buffer[idx]._value.~T(); }

        inline void copy_buffer(const circular_buffer& other)
        {
            const_iterator       first = other.cbegin();
            const const_iterator last  = other.cend();

            for(; first != last; ++first)
                push_back(*first);
        }


#if !defined(JM_CIRCULAR_BUFFER_CXX_OLD)

        inline void move_buffer(circular_buffer&& other)
        {
            iterator       first = other.begin();
            const iterator last  = other.end();

            for(; first != last; ++first)
                emplace_back(std::move(*first));
        }

#endif // !defined(JM_CIRCULAR_BUFFER_CXX_OLD)

    public:
        JM_CB_CONSTEXPR explicit circular_buffer()
            : _head(1), _tail(0), _size(0), _buffer()
        {}

#if defined(JM_CIRCULAR_BUFFER_CXX_OLD)
        explicit
#endif
            circular_buffer(size_type count, const T& value = T())
            : _head(0), _tail(count - 1), _size(count), _buffer()
        {
            if(JM_CB_UNLIKELY(_size > N))
                throw std::out_of_range(
                    "circular_buffer<T, N>(size_type count, const T&) count exceeded N");

            if(JM_CB_LIKELY(_size != 0))
                for(size_type i = 0; i < count; ++i)
                    new(JM_CB_ADDRESSOF(_buffer[i]._value)) T(value);
            else
                _head = 1;
        }

        template<typename InputIt>
        circular_buffer(InputIt first, InputIt last)
            : _head(0), _tail(0), _size(0), _buffer()
        {
            if(first != last) {
                for(; first != last; ++first, ++_size) {
                    if(JM_CB_UNLIKELY(_size >= N))
                        throw std::out_of_range(
                            "circular_buffer<T, N>(InputIt first, InputIt last) distance exceeded N");

                    new(JM_CB_ADDRESSOF(_buffer[_size]._value)) T(*first);
                }

                _tail = _size - 1;
            }
            else
                _head = 1;
        }

#if !defined(JM_CIRCULAR_BUFFER_CXX_OLD)

        circular_buffer(std::initializer_list<T> init)
            : _head(0), _tail(init.size() - 1), _size(init.size()), _buffer()
        {
            if(JM_CB_UNLIKELY(_size > N))
                throw std::out_of_range(
                    "circular_buffer<T, N>(std::initializer_list<T> init) init.size() > N");

            if(JM_CB_UNLIKELY(_size == 0))
                _head = 1;

            storage_type* buf_ptr = _buffer;
            for(auto it = init.begin(), end = init.end(); it != end; ++it, ++buf_ptr)
                new(JM_CB_ADDRESSOF(buf_ptr->_value)) T(*it);
        }

#endif // !defined(JM_CIRCULAR_BUFFER_CXX_OLD)

        circular_buffer(const circular_buffer& other)
            : _head(1), _tail(0), _size(0), _buffer()
        {
            copy_buffer(other);
        }

        circular_buffer& operator=(const circular_buffer& other)
        {
            clear();
            copy_buffer(other);
            return *this;
        }

#if !defined(JM_CIRCULAR_BUFFER_CXX_OLD)

        circular_buffer(circular_buffer&& other) : _head(1), _tail(0), _size(0), _buffer()
        {
            move_buffer(std::move(other));
        }

        circular_buffer& operator=(circular_buffer&& other)
        {
            clear();
            move_buffer(std::move(other));
            return *this;
        }

#endif // !defined(JM_CIRCULAR_BUFFER_CXX_OLD)

        ~circular_buffer() { clear(); }

        /// capacity
        JM_CB_CONSTEXPR bool empty() const JM_CB_NOEXCEPT { return _size == 0; }

        JM_CB_CONSTEXPR bool full() const JM_CB_NOEXCEPT { return _size == N; }

        JM_CB_CONSTEXPR size_type size() const JM_CB_NOEXCEPT { return _size; }

        JM_CB_CONSTEXPR size_type max_size() const JM_CB_NOEXCEPT { return N; }

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
            return JM_CB_ADDRESSOF(_buffer[0]._value);
        }

        JM_CB_CONSTEXPR const_pointer data() const JM_CB_NOEXCEPT
        {
            return JM_CB_ADDRESSOF(_buffer[0]._value);
        }

        /// modifiers
        void push_back(const value_type& value)
        {
            size_type new_tail;
            if(JM_CIRCULAR_BUFFER_FULLNESS_LIKEHOOD(_size == N)) {
                new_tail = _head;
                _head    = wrapper_t::increment(_head);
                --_size;
                _buffer[new_tail]._value = value;
            }
            else {
                new_tail = wrapper_t::increment(_tail);
                new(JM_CB_ADDRESSOF(_buffer[new_tail]._value)) T(value);
            }

            _tail = new_tail;
            ++_size;
        }

        void push_front(const value_type& value)
        {
            size_type new_head = 0;
            if(JM_CIRCULAR_BUFFER_FULLNESS_LIKEHOOD(_size == N)) {
                new_head = _tail;
                _tail    = wrapper_t::decrement(_tail);
                --_size;
                _buffer[new_head]._value = value;
            }
            else {
                new_head = wrapper_t::decrement(_head);
                new(JM_CB_ADDRESSOF(_buffer[new_head]._value)) T(value);
            }

            _head = new_head;
            ++_size;
        }

#if !defined(JM_CIRCULAR_BUFFER_CXX_OLD)

        void push_back(value_type&& value)
        {
            size_type new_tail;
            if(JM_CIRCULAR_BUFFER_FULLNESS_LIKEHOOD(_size == N)) {
                new_tail = _head;
                _head    = wrapper_t::increment(_head);
                --_size;
                _buffer[new_tail]._value = detail::move_if_noexcept_assign(value);
            }
            else {
                new_tail = wrapper_t::increment(_tail);
                new(JM_CB_ADDRESSOF(_buffer[new_tail]._value))
                    T(std::move_if_noexcept(value));
            }

            _tail = new_tail;
            ++_size;
        }

        void push_front(value_type&& value)
        {
            size_type new_head = 0;
            if(JM_CIRCULAR_BUFFER_FULLNESS_LIKEHOOD(_size == N)) {
                new_head = _tail;
                _tail    = wrapper_t::decrement(_tail);
                --_size;
                _buffer[new_head]._value = detail::move_if_noexcept_assign(value);
            }
            else {
                new_head = wrapper_t::decrement(_head);
                new(JM_CB_ADDRESSOF(_buffer[new_head]._value))
                    T(std::move_if_noexcept(value));
            }

            _head = new_head;
            ++_size;
        }

        template<typename... Args>
        void emplace_back(Args&&... args)
        {
            size_type new_tail;
            if(JM_CIRCULAR_BUFFER_FULLNESS_LIKEHOOD(_size == N)) {
                new_tail = _head;
                _head    = wrapper_t::increment(_head);
                --_size;
                destroy(new_tail);
            }
            else
                new_tail = wrapper_t::increment(_tail);

            new(JM_CB_ADDRESSOF(_buffer[new_tail]._value))
                value_type(std::forward<Args>(args)...);
            _tail = new_tail;
            ++_size;
        }

        template<typename... Args>
        void emplace_front(Args&&... args)
        {
            size_type new_head;
            if(JM_CIRCULAR_BUFFER_FULLNESS_LIKEHOOD(_size == N)) {
                new_head = _tail;
                _tail    = wrapper_t::decrement(_tail);
                --_size;
                destroy(new_head);
            }
            else
                new_head = wrapper_t::decrement(_head);

            new(JM_CB_ADDRESSOF(_buffer[new_head]._value))
                value_type(std::forward<Args>(args)...);
            _head = new_head;
            ++_size;
        }

#endif // !defined(JM_CIRCULAR_BUFFER_CXX_OLD)

        JM_CB_CXX14_CONSTEXPR void pop_back() JM_CB_NOEXCEPT
        {
            size_type old_tail = _tail;
            --_size;
            _tail = wrapper_t::decrement(_tail);
            destroy(old_tail);
        }

        JM_CB_CXX14_CONSTEXPR void pop_front() JM_CB_NOEXCEPT
        {
            size_type old_head = _head;
            --_size;
            _head = wrapper_t::increment(_head);
            destroy(old_head);
        }

        JM_CB_CXX14_CONSTEXPR void clear() JM_CB_NOEXCEPT
        {
            while(_size != 0)
                pop_back();

            _head = 1;
            _tail = 0;
        }

        /// iterators
        JM_CB_CXX14_CONSTEXPR iterator begin() JM_CB_NOEXCEPT
        {
            if(_size == 0)
                return end();
            return iterator(_buffer, _head, _size);
        }

        JM_CB_CXX14_CONSTEXPR const_iterator begin() const JM_CB_NOEXCEPT
        {
            if(_size == 0)
                return end();
            return const_iterator(_buffer, _head, _size);
        }

        JM_CB_CXX14_CONSTEXPR const_iterator cbegin() const JM_CB_NOEXCEPT
        {
            if(_size == 0)
                return cend();
            return const_iterator(_buffer, _head, _size);
        }

        JM_CB_CXX14_CONSTEXPR reverse_iterator rbegin() JM_CB_NOEXCEPT
        {
            if(_size == 0)
                return rend();
            return reverse_iterator(iterator(_buffer, _head, _size));
        }

        JM_CB_CXX14_CONSTEXPR const_reverse_iterator rbegin() const JM_CB_NOEXCEPT
        {
            if(_size == 0)
                return rend();
            return const_reverse_iterator(const_iterator(_buffer, _head, _size));
        }

        JM_CB_CXX14_CONSTEXPR const_reverse_iterator crbegin() const JM_CB_NOEXCEPT
        {
            if(_size == 0)
                return crend();
            return const_reverse_iterator(const_iterator(_buffer, _head, _size));
        }

        JM_CB_CXX14_CONSTEXPR iterator end() JM_CB_NOEXCEPT
        {
            return iterator(_buffer, wrapper_t::increment(_tail), 0);
        }

        JM_CB_CXX14_CONSTEXPR const_iterator end() const JM_CB_NOEXCEPT
        {
            return const_iterator(_buffer, wrapper_t::increment(_tail), 0);
        }

        JM_CB_CXX14_CONSTEXPR const_iterator cend() const JM_CB_NOEXCEPT
        {
            return const_iterator(_buffer, wrapper_t::increment(_tail), 0);
        }

        JM_CB_CXX14_CONSTEXPR reverse_iterator rend() JM_CB_NOEXCEPT
        {
            return reverse_iterator(iterator(_buffer, wrapper_t::increment(_tail), 0));
        }

        JM_CB_CXX14_CONSTEXPR const_reverse_iterator rend() const JM_CB_NOEXCEPT
        {
            return const_reverse_iterator(
                const_iterator(_buffer, wrapper_t::increment(_tail), 0));
        }

        JM_CB_CXX14_CONSTEXPR const_reverse_iterator crend() const JM_CB_NOEXCEPT
        {
            return const_reverse_iterator(
                const_iterator(_buffer, wrapper_t::increment(_tail), 0));
        }
    };

} // namespace jm

#endif // include guard
