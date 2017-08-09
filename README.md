# circular_buffer
A simple circular buffer implementation.
Tests and documentation still needs work but I'd say it's working properly and the api is STL style.

## Small example
```c++
#include "circular_buffer.hpp"

// constructors accept iterators, initializer lists or count + element
jm::circular_buffer<int, 4> cb{ 1,2,3 };
cb.push_back(4);  // 1234
cb.push_front(0); // 0123
cb.push_back(5);  // 1235
cb.pop_front(); // 235 also supports pop_back
// iterators are supported and constexpr ( except reverse ones because std::reverse_iterator ) 
for(auto& value : cb)
    std::cout << value; 
cb.size(); // 3
cb.max_size() // 4
cb.clear(); // 
// this can also be done constexpr.
// using c++14 the only non constexpr api is emplace_back and emplace_front
```

## How to use
The library is a single header so all you need to do is copy the header to your directory and include it.

By default it uses c++ 11 features. However you can define JM_CIRCULAR_BUFFER_CXX_14 for most of the circular_buffer to become constexpr or JM_CIRCULAR_BUFFER_CXX_OLD for c++98 ( maybe even lower? ) support.

It is also possible to micro optimize the buffer ( on clang and gcc only ) if you know if it will likely be full or not by using JM_CIRCULAR_BUFFER_LIKELY_FULL OR JM_CIRCULAR_BUFFER_UNLIKELY_FULL.
