#ifndef SB_ALLOCATOR_H
#define SB_ALLOCATOR_H

#include <cstdint>
#include <cstddef>
#include <cstdlib>
#include <cstring>
#include <list>
#include <algorithm>
#include <memory>
#include <stdexcept>

#if (SBA_THREAD_SAFETY)
#include <mutex>
#endif

#include "iterator.h"

struct Segment {
    uintptr_t m_base;
    uintptr_t m_vbase;
    uintptr_t m_limit;
    bool m_mapped;
}; // End of struct

template <typename T>
class SbAllocator : public std::allocator<T> {
public:
    using iterator = Iterator<T, SbAllocator>;
    using pointer = Iterator<T, SbAllocator>;
    using const_pointer = const pointer;
    using size_type = size_t;
    SbAllocator(void);
    SbAllocator(const SbAllocator &other);
    SbAllocator(SbAllocator &&other);
    pointer allocate(size_type n);
    void deallocate(pointer p);
    pointer mmap(T *p, size_type n);
    void unmap(pointer p);
    uintptr_t translateAddr(uintptr_t vaddr);
    iterator begin(void);
    iterator end(void);
    const iterator cbegin(void) const;
    const iterator cend(void) const;
    ~SbAllocator(void);
#if (SBA_PROFILE)
    size_t getCacheHit1(void) const { return m_cache_hit1; }
    size_t getCacheHit2(void) const { return m_cache_hit2; }
    size_t getCacheMiss(void) const { return m_cache_miss; }
#endif
private:
    std::list<Segment> m_seglist;
    uintptr_t m_total_limit;
    Segment *m_cache_segm1;
    Segment *m_cache_segm2;
#if (SBA_THREAD_SAFETY)
    std::mutex m_mutex;
#endif
    static constexpr uintptr_t base_vaddr = 0x100;
#if (SBA_PROFILE)
    size_t m_cache_hit1 = 0;
    size_t m_cache_hit2 = 0;
    size_t m_cache_miss = 0;
#endif
}; // End of class

template <typename T>
SbAllocator<T>::SbAllocator(void)
    : std::allocator<T>()
    , m_total_limit(base_vaddr)
    , m_cache_segm1(nullptr)
    , m_cache_segm2(nullptr)
{}

template <typename T>
SbAllocator<T>::SbAllocator(const SbAllocator &other)
    : std::allocator<T>(other)
    , m_total_limit(base_vaddr)
    , m_cache_segm1(nullptr)
    , m_cache_segm2(nullptr)
{
    for (const auto &x : other.m_seglist) {
        size_type size = (x.m_limit - x.m_vbase) / sizeof(T);
        pointer ptr = allocate(size);
        void *dst = reinterpret_cast<void*>(translateAddr(ptr));
        void *src = reinterpret_cast<void*>(x.m_base);
        std::memcpy(dst, src, size);
    }

    m_total_limit = other.m_total_limit;
}

template <typename T>
SbAllocator<T>::SbAllocator(SbAllocator &&other)
    : std::allocator<T>(other)
    , m_seglist(other.m_seglist)
    , m_cache_segm1(other.m_cache_segm1)
    , m_cache_segm2(other.m_cache_segm2)
{
    other.m_seglist.clear();
    other.m_total_limit = base_vaddr;
    other.m_cache_segm1 = nullptr;
    other.m_cache_segm2 = nullptr;
}

template <typename T>
SbAllocator<T>::~SbAllocator(void)
{
#if (SBA_THREAD_SAFETY)
    std::lock_guard<std::mutex> lock(m_mutex);
#endif

    auto f = [] (const Segment &s)
        { if (!s.m_mapped) { std::free(reinterpret_cast<void*>(s.m_base)); } };

    std::for_each(m_seglist.begin(), m_seglist.end(), f);
}

template <typename T>
typename SbAllocator<T>::pointer SbAllocator<T>::allocate(size_type n)
{
    size_type size = n * sizeof(T);

    uintptr_t ptr = reinterpret_cast<uintptr_t>(std::malloc(size));
    if (!ptr) {
        throw std::bad_alloc();
    }

#if (SBA_THREAD_SAFETY)
    std::lock_guard<std::mutex> lock(m_mutex);
#endif

    Segment segm = {ptr, m_total_limit, m_total_limit + size, false};

    m_seglist.emplace_back(segm);

    Iterator<T, SbAllocator> temp(this, m_total_limit);

    m_total_limit += size;

    return temp;
}

template <typename T>
void SbAllocator<T>::deallocate(pointer p)
{
#if (SBA_THREAD_SAFETY)
    std::lock_guard<std::mutex> lock(m_mutex);
#endif

    auto f1 = [p = p] (const Segment &s) { return (s.m_vbase == p); };
    auto iter = std::find_if(m_seglist.begin(), m_seglist.end(), f1);
    if (iter == m_seglist.end()) {
        throw std::runtime_error(
            "SbAllocator<T>::deallocate(pointer) failed, invalid pointer");
    }

    size_type size = iter->m_limit - iter->m_vbase;

    if (m_cache_segm1 == std::addressof(*iter)) {
        m_cache_segm1 = nullptr;
        m_cache_segm2 = nullptr;
    }

    if (m_cache_segm2 == std::addressof(*iter)) {
        m_cache_segm2 = nullptr;
    }

    auto f2 = [size = size] (Segment &s)
        { s.m_vbase -= size; s.m_limit -= size; return; };

    std::for_each(iter, m_seglist.end(), f2);

    m_total_limit -= size;

    std::free(reinterpret_cast<void*>(iter->m_base));
    m_seglist.erase(iter);
}

template <typename T>
typename SbAllocator<T>::pointer SbAllocator<T>::mmap(T *p, size_type n)
{
    size_type size = n * sizeof(T);
    uintptr_t ptr = reinterpret_cast<uintptr_t>(p);

#if (SBA_THREAD_SAFETY)
    std::lock_guard<std::mutex> lock(m_mutex);
#endif

    Segment segm = {ptr, m_total_limit, m_total_limit + size, true};

    m_seglist.emplace_back(segm);

    Iterator<T, SbAllocator> temp(this, m_total_limit);

    m_total_limit += size;

    return temp;
}

template <typename T>
void SbAllocator<T>::unmap(pointer p)
{
#if (SBA_THREAD_SAFETY)
    std::lock_guard<std::mutex> lock(m_mutex);
#endif

    auto f = [p = p] (const Segment &s) { return (s.m_vbase == p); };
    auto iter = std::find_if(m_seglist.begin(), m_seglist.end(), f);
    if (iter == m_seglist.end()) {
        throw std::runtime_error(
            "SbAllocator<T>::unmap(pointer) failed, invalid pointer");
    }

    size_type size = iter->m_limit - iter->m_vbase;

    if (m_cache_segm1 == std::addressof(*iter)) {
        m_cache_segm1 = nullptr;
        m_cache_segm2 = nullptr;
    }

    if (m_cache_segm2 == std::addressof(*iter)) {
        m_cache_segm2 = nullptr;
    }

    auto f2 = [size = size] (Segment &s)
        { s.m_vbase -= size; s.m_limit -= size; return; };

    std::for_each(iter, m_seglist.end(), f2);

    m_total_limit -= size;

    m_seglist.erase(iter);
}

template <typename T>
uintptr_t SbAllocator<T>::translateAddr(uintptr_t vaddr)
{
    if (vaddr >= m_total_limit) {
        throw std::out_of_range(
            std::string("Iterator::at: __n (which is " +
            std::to_string(vaddr + 1) + ") >= _Nm (which is " +
            std::to_string(m_total_limit) + ")"));
    }

    if (m_cache_segm1) {
        if (m_cache_segm1->m_vbase <= vaddr && m_cache_segm1->m_limit > vaddr) {
#if (SBA_PROFILE)
            m_cache_hit1 += 1;
#endif
            return m_cache_segm1->m_base + (vaddr - m_cache_segm1->m_vbase);
        }
    }

    if (m_cache_segm2) {
        if (m_cache_segm2->m_vbase <= vaddr && m_cache_segm2->m_limit > vaddr) {
#if (SBA_PROFILE)
            m_cache_hit2 += 1;
#endif
            m_cache_segm1 = m_cache_segm2;
            return m_cache_segm2->m_base + (vaddr - m_cache_segm2->m_vbase);
        }
    }

#if (SBA_PROFILE)
    m_cache_miss += 1;
#endif

    auto f = [vaddr = vaddr] (const Segment &s)
        { return (s.m_vbase <= vaddr && s.m_limit > vaddr); };

    auto iter = std::find_if(m_seglist.begin(), m_seglist.end(), f);
    if (iter == m_seglist.end()) {
        // unreachable
        throw std::runtime_error("SbAllocator<T>::translateAddr(uintptr_t) failed");
    }

    uintptr_t res = iter->m_base + (vaddr - iter->m_vbase);

    m_cache_segm1 = std::addressof(*iter);

    if (++iter != m_seglist.end()) {
        m_cache_segm2 = std::addressof(*iter);
    }

    return res;
}

template <typename T>
typename SbAllocator<T>::iterator SbAllocator<T>::begin(void)
{
    return Iterator<T, SbAllocator>(this, base_vaddr);
}

template <typename T>
typename SbAllocator<T>::iterator SbAllocator<T>::end(void)
{
    return Iterator<T, SbAllocator>(this, m_total_limit);
}

template <typename T>
const typename SbAllocator<T>::iterator SbAllocator<T>::cbegin(void) const
{
    return Iterator<T, SbAllocator>(this, base_vaddr);
}

template <typename T>
const typename SbAllocator<T>::iterator SbAllocator<T>::cend(void) const
{
    return Iterator<T, SbAllocator>(this, m_total_limit);
}

template <typename T>
constexpr uintptr_t SbAllocator<T>::base_vaddr;

#endif // SB_ALLOCATOR_H
