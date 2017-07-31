#ifndef JM_CIRCULAR_BUFFER_HPP
#define JM_CIRCULAR_BUFFER_HPP

#include <iterator>

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

        void increment() noexcept
        {
            _pos = (_pos + 1) % N;
        }

        void decrement() noexcept
        {
            return (_pos + N) % (N);
        }

    public:
        circular_buffer_iterator() = default;
        explicit circular_buffer_iterator(pointer buf, std::size_t pos)
            : _buf(buf)
            , _pos(pos)
        {}

        reference operator*() const
        {
            return *(_buf + _pos);
        }
        pointer operator->() const
        {
            return _buf + _pos;
        }

        circular_buffer_iterator& operator++()
        {
            increment();
            return *this;
        }
        circular_buffer_iterator& operator--()
        {
            decrement();
            return *this;
        }

        circular_buffer_iterator operator++(int)
        {
            auto temp = *this;
            increment();
            return temp;
        }
        circular_buffer_iterator operator--(int)
        {
            auto temp = *this;
            decrement();
            return temp;
        }

        template<typename Tx>
        bool operator==(const circular_buffer_iterator<Tx, N>& lhs)
        {
            return lhs._pos == _pos && lhs._buf == _buf;
        }

        template<typename Tx>
        bool operator!=(const circular_buffer_iterator<Tx, N>& lhs)
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
    
    public:
        circular_buffer()
            : _head(1)
            , _tail(0)
            , _size(0)
        {}

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
        void push_back(const value_type& value)
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
        
        void push_back(value_type&& value)
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
        
        void push_front(const value_type& value)
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
        
        void push_front(value_type&& value)
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
        
        void pop_back() noexcept
        {
            auto old_tail = _tail;
            _tail = decrement(_tail);
            _buffer[old_tail].~value_type();
        }
        
        void pop_front() noexcept
        {
            auto old_head = _head;
            _head = decrement(_head);
            _buffer[old_head].~value_type();
        }

        void clear() noexcept
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
            return iterator(_buffer, _head);
        }

        const_iterator begin() const noexcept
        {
            if (_size == 0)
                return end();
            return const_iterator(_buffer, _head);
        }

        const_iterator cbegin() const noexcept
        {
            if (_size == 0)
                return cend();
            return const_iterator(_buffer, _head);
        }

        reverse_iterator rbegin() noexcept
        {
            if (_size == 0)
                return rend();
            return reverse_iterator(iterator{ _buffer, _head });
        }

        const_reverse_iterator  rbegin() const noexcept
        {
            if (_size == 0)
                return rend();
            return const_reverse_iterator(const_iterator{ _buffer, _head });
        }

        const_reverse_iterator  crbegin() const noexcept
        {
            if (_size == 0)
                return crend();
            return const_reverse_iterator(const_iterator{ _buffer, _head });
        }

        iterator end() noexcept
        {
            return iterator(_buffer, _tail);
        }

        const_iterator end() const noexcept
        {
            return const_iterator(_buffer, _tail);
        }

        const_iterator cend() const noexcept
        {
            return const_iterator(_buffer, _tail);
        }

        reverse_iterator rend() noexcept
        {
            return reverse_iterator(iterator{ _buffer, _tail });
        }

        const_reverse_iterator rend() const noexcept
        {
            return const_reverse_iterator(const_iterator{ _buffer, _tail });
        }

        const_reverse_iterator crend() const noexcept
        {
            return const_reverse_iterator(const_iterator{ _buffer, _tail });
        }
    };

}

#endif JM_CIRCULAR_BUFFER_HPP
