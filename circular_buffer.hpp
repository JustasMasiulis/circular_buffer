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
        
        constexpr size_type increment(size_type idx) const noexcept
        {
            return (idx + 1) % N;
        }
        
        constexpr size_type decrement(size_type idx) const noexcept
        {
            return (idx + N) % (N + 1);
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
        
        template<typename... Args>
        void emplace( Args&&... args );
        {
            auto new_tail = (_tail + 1) % N;
            if(_size == N){
                inc_head();
                _buffer[new_tail].~value_type();
            }
          
            new (&_buffer[new_tail]) value_type(std::forward<Args>(args)...); 
            _tail = new_tail;
            ++_size;
        }
        
    };

}

#endif JM_CIRCULAR_BUFFER_HPP
