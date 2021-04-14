#include <cstdio>
#include <cstdint>
#include <cstddef>

#include "sb_allocator.h"


int main(void)
{
    SbAllocator<uint32_t> a2;
    a2.allocate(2);

    uint32_t *buf1 = new uint32_t [4];
    auto p1 = a2.mmap(buf1, 4);

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

    a2.unmap(p1);
    delete[] buf1;

    for (const auto &x : a2) {
        printf("%u ", x);
    }
    printf("\n");

    return 0;
}
