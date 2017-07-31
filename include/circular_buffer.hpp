#ifndef JM_CIRCULAR_BUFFER_HPP
#define JM_CIRCULAR_BUFFER_HPP

#include <iterator>
#include <list>

namespace jm
{



    template<typename T>
    class circular_buffer_iterator
        : public std::iterator<std::bidirectional_iterator_tag
                              , typename T::value_type
                              , typename T::difference_type
                              , typename T::pointer
                              , typename T::reference>
    {
        pointer _buf;
        pointer _cur;

        void increment()
        {
            _cur = _buf + (_buf - _cur) % T{}.max_size();
        }

    public:
        circular_buffer_iterator(const circular_buffer_iterator&);
        ~circular_buffer_iterator();
        circular_buffer_iterator& operator=(const circular_buffer_iterator&);



        iterator& operator--(); //prefix increment
        iterator operator--(int); //postfix decrement

        circular_buffer_iterator& operator++(); //prefix increment
        {
            ++_cur;
            return *this;
        }

        circular_buffer_iterator operator++(int); //postfix increment
        {

        }



        reference operator*() const
        {
            return *_cur;
        }

        value_type operator*() const
        {
            return *_cur;
        }

        pointer operator->() const
        {
            return _cur;
        }

        friend bool operator==(const iterator&, const iterator&);
        friend bool operator!=(const iterator&, const iterator&);
        friend void swap(iterator& lhs, iterator& rhs); //C++11 I think

    };

    template<typename T, std::size_t N>
    class circular_buffer
    {
    public: 
        using value_type             = T; 
        using size_type              = std::size_t;
        using difference_type        = std::ptrdiff_t;
        using reference_type         = value_type&;
        using const_reference_type   = const value_type&;
        using pointer                = value_type*;
        using const_pointer          = const value_type*;
        using iterator               = ;
        using const_iterator         = ;
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
            return (idx + N) % (N + 1);
        }

        constexpr void destroy(size_type idx) noexcept
        {
            _buffer[idx].~T();
        }

        template<typename... Args>
        inline void construct(size_type idx, Args&&... args)
        {
            new (&_buffer[new_tail]) T(std::forward<Args>(args)...);
        }
    
    public:
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

        constexpr T* data() noexcept
        {
            return &_buffer;
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

            std::list<int> v;
            v.begin()
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
        }
        
    };

}

#endif JM_CIRCULAR_BUFFER_HPP
