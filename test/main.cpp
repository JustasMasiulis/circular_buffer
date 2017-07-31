#include "../include/circular_buffer.hpp"

int main() {

    jm::circular_buffer<int, 5> cb;
    cb.front();
    cb.back();
    cb.empty();
    cb.full();
    cb.data();
    cb.emplace_back(5);
    cb.push_back(1);
    cb.push_front(2);
    cb.max_size();
    cb.pop_front();
    cb.pop_back();
    cb.size();
    cb.clear();

    cb.begin();
    cb.cbegin();

    cb.end();
    cb.cend();
}