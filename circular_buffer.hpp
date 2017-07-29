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
        size_type        _first;
        size_type        _size;
        std::array<T, N> _buffer;
        
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
            return _buffer[_first];
        }
        
        constexpr const_reference_type front() const noexcept
        {
            return _buffer[_first];
        }
        
        constexpr reference_type back() noexcept
        {
            return _buffer[mask(_first + _size - 1)];
        }
        
        constexpr const_reference_type back() const noexcept
        {
            return _buffer[mask(_first + _size - 1)];
        }
        
        /// modifiers
        constexpr void push(const value_type& value)
        {
            _buffer[mask(_first + _size)] = value;
            push();
        }
        
        constexpr void push(value_type&& value)
        {
            _buffer[mask(_first + _size)] = std::move(value);
            push();
        }
        
        constexpr pop()
        {
            back.~value_type();
            --_size;
        }
        template< class... Args >
        void emplace( Args&&... args );
        {
        }
        
    };

}

#endif JM_CIRCULAR_BUFFER_HPP
