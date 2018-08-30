# std::vector<std::uint2_t>

std::vector<std::uint2_t> is a space-efficient template specializations of standard library [std::vector](https://en.cppreference.com/w/cpp/container/vector) for the 2-bit values.

## Example Usage
```c++
#include <iostream>
#include <vector>
#include "uint2_t.hpp"

int main()
{
    std::vector<std::uint2_t> v = {0, 1, 2, 3, 2, 1, 0};
    for (auto i : v)
        std::cout << static_cast<unsigned>(i) << ' ';
    std::cout << '\n';
}
```