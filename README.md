# circular_buffer
A simple circular buffer implementation.
Tests and documentation still needs work but I'd say it's working properly and the api is STL style.

## How to use
The library is a single header so all you need to do is copy the header to your directory and include it.

By default it uses c++ 11 features. However you can define JM_CIRCULAR_BUFFER_CXX_14 for most of the circular_buffer to become constexpr or JM_CIRCULAR_BUFFER_CXX_OLD for c++98 ( maybe even lower? ) support.

It is also possible to micro optimize the buffer ( on clang and gcc only ) if you know if it will likely be full or not by using JM_CIRCULAR_BUFFER_LIKELY_FULL OR JM_CIRCULAR_BUFFER_UNLIKELY_FULL.
