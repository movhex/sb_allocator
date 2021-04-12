#include <cstdio>
#include <cstdint>
#include <cstddef>

#include "sb_allocator.h"


int main(void)
{
    SbAllocator<char> a1;

    a1.allocate(1);
    a1.allocate(3);
    a1.allocate(5);

    char c = 97;

    for (auto i = a1.begin(); i != a1.end(); ++i) {
        *i = c;
        c++;
    }
    for (auto i = a1.begin(); i != a1.end(); ++i) {
        printf("%c ", *i);
    }
    printf("\n");

    SbAllocator<uint32_t> a2;
    a2.allocate(2);
    auto p1 = a2.allocate(4);
    a2.allocate(3);

    uint32_t d = 0;

    for (auto &x : a2) {
        x = d;
        d++;
    }

    for (const auto &x : a2) {
        printf("%u ", x);
    }
    printf("\n");

    a2.deallocate(p1);

    for (const auto &x : a2) {
        printf("%u ", x);
    }
    printf("\n");

    return 0;
}
