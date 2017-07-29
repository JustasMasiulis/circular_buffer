#ifndef JM_CIRCULAR_BUFFER_HPP
#define JM_CIRCULAR_BUFFER_HPP

namespace jm
{

    template<typename T, std::size_t N>
    class circular_buffer
    {
    public: 
        using value_type           = T; 
        using size_type            = std::size_t;
        using reference_type       = value_type&;
        using const_reference_type = const value_type&;
        
    private:
        size_type        _head;
        size_type        _tail;
        size_type        _size;
        std::array<T, N> _buffer;
        
        constexpr void inc_tail() noexcept
        {
            tail = (tail + 1) % N; 
            ++_size;
        }
        
        constexpr void inc_head() noexcept
        {
            head = (head + 1) % N; 
            --_size;
        }
        
        constexpr size_type mask(size_type idx) const noexcept
        {
            return idx % (N - 1);
        }
        
        constexpr void push() noexcept
        {
            if(_size == N)
                _first = (_first + 1) % N;
            else
                ++_size;
        }
    
    public:
    /// capacity
        constexpr bool empty() const noexcept
        {
            return _size == 0;
        }
        
        constexpr size_type size() const noexcept
        {
            return _size;
        }
        
        constexpr max_size() const noexcept
        {
            return N; 
        }
       
    /// element access
        constexpr reference_type front() noexcept
        {
            return _buffer[_head];
        }
        
        constexpr const_reference_type front() const noexcept
        {
            return _buffer[_head];
        }
        
        constexpr reference_type back() noexcept
        {
            return _buffer[_tail];
        }
        
        constexpr const_reference_type back() const noexcept
        {
            return _buffer[_tail];
        }
        
        /// modifiers
        void push(const value_type& value)
        {
            auto new_tail = (_tail + 1) % N;
            if(_size == N)
                inc_head();
            _buffer[new_tail] = value; 
            _tail = new_tail;
            ++_size;
        }
        
        void push(value_type&& value)
        {
            auto new_tail = (_tail + 1) % N;
            if(_size == N)
                inc_head();
            _buffer[new_tail] = std::move(value); 
            _tail = new_tail;
            ++_size;
        }
        
        void pop() noexcept
        {
            inc_head();
            back.~value_type();
        }
        
        template<typename... Args>
        void emplace( Args&&... args );
        {
            auto new_tail = (_tail + 1) % N;
            if(_size == N)
                inc_head();
            _buffer[new_tail].~value_type();
            new (&_buffer[new_tail]) value_type(std::forward<Args>(args)...); 
            _tail = new_tail;
            ++_size;
        }
        
    };

}

#endif JM_CIRCULAR_BUFFER_HPP
