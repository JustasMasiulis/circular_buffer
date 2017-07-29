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
        size_type        _last;
        std::array<T, N> _buffer;
    
    public:
    /// capacity
        constexpr bool empty() const noexcept
        {
            return _first == _last;
        }
        
        constexpr size_type size() const noexcept
        {
            return _first > _last
                 ? _first - _last
                 : _last  - _first;
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
            return _buffer[_last];
        }
        
        constexpr const_reference_type back() const noexcept
        {
            return _buffer[_last];
        }
        
    };

}

#endif JM_CIRCULAR_BUFFER_HPP
