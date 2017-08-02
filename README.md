# circular_buffer
A simple circular buffer implementation.
Tests and documentation still needs work but I'd say it's working properly and the api is STL style.

# how to use
The library is header only but needs boost.config that is included as submodule.
If you do choose to use boost.config it only needs to be added to include directories.
If you want to use the library without boost dependency you need to define JM_CIRCULAR_BUFFER_NO_BOOST.
Without the boost dependency all you need to do is include the circular_buffer.hpp to your code and thats it however the library will default to c++11 features.
By defining JM_CIRCULAR_BUFFER_CXX_14 you will make most of the constructors and functions constexpr.
It is also possible to use this library even in c++98 by defining JM_CIRCULAR_BUFFER_CXX_OLD.
