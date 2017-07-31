#ifndef JM_CIRCULAR_BUFFER_HPP
#define JM_CIRCULAR_BUFFER_HPP

#include <iterator>
#include <algorithm>
#include <initializer_list>

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

        constexpr void increment() noexcept
        {
            _pos = (_pos + 1) % N;
        }

        constexpr void decrement() noexcept
        {
            _pos = (_pos + N - 1) % (N);
        }

    public:
        explicit constexpr circular_buffer_iterator() noexcept
            : _buf(nullptr)
            , _pos(0)
            , _left_in_forward(0)
        {}

        explicit constexpr circular_buffer_iterator(pointer buf, std::size_t pos, std::size_t left_in_forward) noexcept
            : _buf(buf)
            , _pos(pos)
            , _left_in_forward(left_in_forward)
        {}

        template<typename Tnc>
        explicit constexpr circular_buffer_iterator(const circular_buffer_iterator<Tnc, N> lhs)
            : _buf(lhs._buf)
            , _pos(lhs._pos)
        {}

        constexpr reference operator*() const noexcept
        {
            return *(_buf + _pos);
        }

        constexpr pointer operator->() const noexcept
        {
            return _buf + _pos;
        }

        circular_buffer_iterator& operator++() noexcept
        {
            increment();
            --_left_in_forward;
            return *this;
        }

        circular_buffer_iterator& operator--() noexcept
        {
            decrement();
            ++_left_in_forward;
            return *this;
        }

        circular_buffer_iterator operator++(int) noexcept
        {
            auto temp = *this;
            increment();
            --_left_in_forward;
            return temp;
        }

        circular_buffer_iterator operator--(int) noexcept
        {
            auto temp = *this;
            decrement();
            ++_left_in_forward;
            return temp;
        }

        template<typename Tx>
        constexpr bool operator==(const circular_buffer_iterator<Tx, N>& lhs) const noexcept
        {
            return lhs._pos == _pos && lhs._left_in_forward == _left_in_forward && lhs._buf == _buf;
        }

        template<typename Tx>
        constexpr bool operator!=(const circular_buffer_iterator<Tx, N>& lhs) const noexcept
        {
            return !(operator==(lhs));
        }
    };
 

    

    template<typename T, std::size_t N>
    class circular_buffer
    {
    public: 
        using value_type             = T; 
        using size_type              = std::size_t;
        using difference_type        = std::ptrdiff_t;
        using reference              = value_type&;
        using const_reference        = const value_type&;
        using pointer                = value_type*;
        using const_pointer          = const value_type*;
        using iterator               = circular_buffer_iterator<T, N>;
        using const_iterator         = circular_buffer_iterator<const T, N>;
        using reverse_iterator       = std::reverse_iterator<iterator>;
        using const_reverse_iterator = std::reverse_iterator<const_iterator>;
        
    private:
        size_type        _head;
        size_type        _tail;
        size_type        _size;
        T                _buffer[N];
        
        constexpr size_type increment(size_type idx) const noexcept
        {
            return (idx + 1) % N;
        }
        
        constexpr size_type decrement(size_type idx) const noexcept
        {
            return (idx + N - 1) % (N);
        }

        constexpr void destroy(size_type idx) noexcept
        {
            _buffer[idx].~T();
        }

        template<typename... Args>
        inline void construct(size_type idx, Args&&... args)
        {
            new (&_buffer[idx]) T(std::forward<Args>(args)...);
        }

        inline void copy_buffer(const T* buffer)
        {
            if (_size == N)
                std::copy(buffer, buffer + N, ::std::addressof(_buffer[0]));
            else if (_head > _tail)
                std::copy(buffer + _tail, buffer + _head + 1, ::std::addressof(_buffer[0]) + _tail);
            else
                std::copy(buffer + _head, buffer + _tail + 1, ::std::addressof(_buffer[0]) + _head);
        }

        inline void move_buffer(T* buffer)
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

        circular_buffer(circular_buffer&& other)
            : _head(other._head)
            , _tail(other._tail)
            , _size(other._size)
        {
            move_buffer(::std::addressof(other._buffer[0]));
        }

        constexpr circular_buffer(std::initializer_list<T> init)
            : _size(init.size())
            , _head(0)
            , _tail(_size - 1)
        {
            if (_size > N)
                throw std::length_error("circular_buffer<T, N>(std::initializer_list<T> init) init.size() > N");

            std::move(init.begin(), init.end(), ::std::addressof(_buffer[0]));
        }

        circular_buffer& operator=(const circular_buffer& other)
        {
            _head = other._head;
            _tail = other._tail;
            _size = other._size;

            copy_buffer(::std::addressof(other._buffer[0]));

            return *this;
        }

        circular_buffer& operator=(circular_buffer&& other)
        {
            _head = other._head;
            _tail = other._tail;
            _size = other._size;

            move_buffer(::std::addressof(other._buffer[0]));

            return *this;
        }

    /// capacity
        constexpr bool empty() const noexcept
        {
            return _size == 0;
        }

        constexpr bool full() const noexcept
        {
            return _size == N;
        }
        
        constexpr size_type size() const noexcept
        {
            return _size;
        }
        
        constexpr size_type max_size() const noexcept
        {
            return N; 
        }
       
    /// element access
        constexpr reference front() noexcept
        {
            return _buffer[_head];
        }
        
        constexpr const_reference front() const noexcept
        {
            return _buffer[_head];
        }
        
        constexpr reference back() noexcept
        {
            return _buffer[_tail];
        }
        
        constexpr const_reference back() const noexcept
        {
            return _buffer[_tail];
        }

        constexpr T* data() noexcept
        {
            return _buffer;
        }

        constexpr const T* data() const noexcept
        {
            return &_buffer;
        }
        
    /// modifiers
        constexpr void push_back(const value_type& value)
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
        
        constexpr void push_back(value_type&& value)
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
        
        constexpr void push_front(const value_type& value)
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
        
        constexpr void push_front(value_type&& value)
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
        
        constexpr void pop_back() noexcept
        {
            auto old_tail = _tail;
            _tail = decrement(_tail);
            _buffer[old_tail].~value_type();
        }
        
        constexpr void pop_front() noexcept
        {
            auto old_head = _head;
            _head = decrement(_head);
            _buffer[old_head].~value_type();
        }

        constexpr void clear() noexcept
        {
            while (_size != 0)
                pop_back();

            _head = 1;
            _tail = 0;
        }

    /// iterators
        iterator begin() noexcept
        {
            if (_size == 0)
                return end();
            return iterator(_buffer, _head, _size);
        }

        const_iterator begin() const noexcept
        {
            if (_size == 0)
                return end();
            return const_iterator(_buffer, _head, _size);
        }

        const_iterator cbegin() const noexcept
        {
            if (_size == 0)
                return cend();
            return const_iterator(_buffer, _head, _size);
        }

        reverse_iterator rbegin() noexcept
        {
            if (_size == 0)
                return rend();
            return reverse_iterator(iterator{ _buffer, _head, _size });
        }

        const_reverse_iterator  rbegin() const noexcept
        {
            if (_size == 0)
                return rend();
            return const_reverse_iterator(const_iterator{ _buffer, _head, _size });
        }

        const_reverse_iterator  crbegin() const noexcept
        {
            if (_size == 0)
                return crend();
            return const_reverse_iterator(const_iterator{ _buffer, _head, _size});
        }

        iterator end() noexcept
        {
            return iterator(_buffer, increment(_tail), 0);
        }

        const_iterator end() const noexcept
        {
            return const_iterator(_buffer, increment(_tail), 0);
        }

        const_iterator cend() const noexcept
        {
            return const_iterator(_buffer, increment(_tail), 0);
        }

        reverse_iterator rend() noexcept
        {
            return reverse_iterator(iterator{ _buffer, increment(_tail), 0 });
        }

        const_reverse_iterator rend() const noexcept
        {
            return const_reverse_iterator(const_iterator{ _buffer, increment(_tail), 0 });
        }

        const_reverse_iterator crend() const noexcept
        {
            return const_reverse_iterator(const_iterator{ _buffer, increment(_tail), 0 });
        }
    };

}

#endif JM_CIRCULAR_BUFFER_HPP
