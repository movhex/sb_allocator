#ifndef ITERATOR_H
#define ITERATOR_H

#include <cstdint>
#include <cstddef>
#include <list>
#include <string>
#include <iterator>
#include <stdexcept>

template <typename T, typename Allocator>
class Iterator;

template <typename T, typename Allocator>
constexpr Iterator<T, Allocator> operator+(const Iterator<T, Allocator> &lhs, size_t n);

template <typename T, typename Allocator>
constexpr Iterator<T, Allocator> operator+(size_t n, const Iterator<T, Allocator> &rhs);

template <typename T, typename Allocator>
constexpr Iterator<T, Allocator> operator-(const Iterator<T, Allocator> &lhs, size_t n);

template <typename T, typename Allocator>
constexpr Iterator<T, Allocator> operator-(size_t n, const Iterator<T, Allocator> &rhs);

template <typename T, typename Allocator>
class Iterator {
    friend constexpr Iterator operator+<>(const Iterator &lhs, size_t n);
    friend constexpr Iterator operator+<>(size_t n, const Iterator &rhs);
    friend constexpr Iterator operator-<>(const Iterator &lhs, size_t n);
    friend constexpr Iterator operator-<>(size_t n, const Iterator &rhs);
public:
    using iterator_category = std::bidirectional_iterator_tag;
    using difference_type = ptrdiff_t;
    using value_type = T;
    using pointer = value_type *;
    using reference = value_type &;
    Iterator(Allocator *m_allocator, uintptr_t vaddr = 0);
    Iterator(const Iterator &other);
    constexpr const Iterator operator=(const Iterator &other);
    constexpr pointer operator->(void) const;
    constexpr reference operator*(void) const;
    constexpr reference operator[](size_t n) const;
    constexpr Iterator operator++(void);
    constexpr Iterator operator++(int);
    constexpr Iterator operator--(void);
    constexpr Iterator operator--(int);
    constexpr Iterator operator+=(size_t n);
    constexpr Iterator operator-=(size_t n);
    constexpr Iterator operator+(const Iterator &other) const;
    constexpr Iterator operator-(const Iterator &other) const;
    constexpr operator size_t(void) const;
    constexpr Iterator next(size_t n = 1);
    constexpr Iterator prev(size_t n = 1);
    constexpr bool operator==(const Iterator &other) const;
    constexpr bool operator!=(const Iterator &other) const;
    constexpr bool operator>(const Iterator &other) const;
    constexpr bool operator<(const Iterator &other) const;
    constexpr bool operator<=(const Iterator &other) const;
    constexpr bool operator>=(const Iterator &other) const;
private:
    uintptr_t m_vaddr;
    Allocator *m_allocator;
}; // End of class


template <typename T, typename Allocator>
Iterator<T, Allocator>::Iterator(Allocator *allocator, uintptr_t vaddr)
    : m_vaddr(vaddr)
    , m_allocator(allocator)
{}

template <typename T, typename Allocator>
Iterator<T, Allocator>::Iterator(const Iterator &other)
    : m_vaddr(other.m_vaddr)
    , m_allocator(other.m_allocator)
{}

template <typename T, typename Allocator>
constexpr const Iterator<T, Allocator> Iterator<T, Allocator>::operator=(const Iterator &other)
{
    if (this == &other) {
        return *this;
    }

    m_vaddr = other.m_vaddr;
    m_allocator = other.m_allocator;
    return *this;
}

template <typename T, typename Allocator>
constexpr typename Iterator<T, Allocator>::pointer Iterator<T, Allocator>::operator->(void) const
{
    if (m_allocator) {
        return reinterpret_cast<T*>(m_allocator->translateAddr(m_vaddr));
    }
    else {
        throw std::runtime_error("Iterator<T, Allocator>::operator->() failed, tried to dereference empty m_allocator");
    }
}

template <typename T, typename Allocator>
constexpr typename Iterator<T, Allocator>::reference Iterator<T, Allocator>::operator*(void) const
{
    pointer p = operator->();
    return *p;
}

template <typename T, typename Allocator>
constexpr typename Iterator<T, Allocator>::reference Iterator<T, Allocator>::operator[](size_t n) const
{
    if (m_allocator) {
        return *reinterpret_cast<T*>(m_allocator->translateAddr(m_vaddr + sizeof(T) * n));
    }
    else {
        throw std::runtime_error("Iterator<T, Allocator>::operator->() failed, tried to dereference empty m_allocator");
    }
}

template <typename T, typename Allocator>
constexpr Iterator<T, Allocator> Iterator<T, Allocator>::operator++(void)
{
    m_vaddr += sizeof(T);
    return *this;
}

template <typename T, typename Allocator>
constexpr Iterator<T, Allocator> Iterator<T, Allocator>::operator++(int)
{
    Iterator<T, Allocator> temp(*this);
    m_vaddr += sizeof(T);
    return temp;
}

template <typename T, typename Allocator>
constexpr Iterator<T, Allocator> Iterator<T, Allocator>::operator--(void)
{
    m_vaddr -= sizeof(T);
    return *this;
}

template <typename T, typename Allocator>
constexpr Iterator<T, Allocator> Iterator<T, Allocator>::operator--(int)
{
    Iterator<T, Allocator> temp(*this);
    m_vaddr -= sizeof(T);
    return temp;
}

template <typename T, typename Allocator>
constexpr Iterator<T, Allocator> Iterator<T, Allocator>::operator+=(size_t n)
{
    m_vaddr += n * sizeof(T);
    return *this;
}

template <typename T, typename Allocator>
constexpr Iterator<T, Allocator> Iterator<T, Allocator>::operator-=(size_t n)
{
    m_vaddr -= n * sizeof(T);
    return *this;
}

template <typename T, typename Allocator>
constexpr Iterator<T, Allocator> Iterator<T, Allocator>::operator+(const Iterator<T, Allocator> &other) const
{
    return Iterator<T, Allocator>(m_allocator, m_vaddr + other.m_vaddr);
}

template <typename T, typename Allocator>
constexpr Iterator<T, Allocator> Iterator<T, Allocator>::operator-(const Iterator<T, Allocator> &other) const
{
    return Iterator<T, Allocator>(m_allocator, m_vaddr - other.m_vaddr);
}

template <typename T, typename Allocator>
constexpr Iterator<T, Allocator>::operator size_t(void) const
{
    return m_vaddr;
}

template <typename T, typename Allocator>
constexpr bool Iterator<T, Allocator>::operator==(const Iterator &other) const
{
    return (m_vaddr == other.m_vaddr);
}

template <typename T, typename Allocator>
constexpr bool Iterator<T, Allocator>::operator!=(const Iterator &other) const
{
    return !(m_vaddr == other.m_vaddr);
}

template <typename T, typename Allocator>
constexpr bool Iterator<T, Allocator>::operator>(const Iterator &other) const
{
    return (m_vaddr > other.m_vaddr);
}

template <typename T, typename Allocator>
constexpr bool Iterator<T, Allocator>::operator<(const Iterator &other) const
{
    return (m_vaddr < other.m_vaddr);
}

template <typename T, typename Allocator>
constexpr bool Iterator<T, Allocator>::operator<=(const Iterator &other) const
{
    return (m_vaddr <= other.m_vaddr);
}

template <typename T, typename Allocator>
constexpr bool Iterator<T, Allocator>::operator>=(const Iterator &other) const
{
    return (m_vaddr >= other.m_vaddr);
}

template <typename T, typename Allocator>
constexpr Iterator<T, Allocator> Iterator<T, Allocator>::next(size_t n)
{
    m_vaddr += n * sizeof(T);
    return *this;
}

template <typename T, typename Allocator>
constexpr Iterator<T, Allocator> Iterator<T, Allocator>::prev(size_t n)
{
    m_vaddr -= n * sizeof(T);
    return *this;
}

template <typename T, typename Allocator>
constexpr Iterator<T, Allocator> operator+(const Iterator<T, Allocator> &lhs, size_t n)
{
    return Iterator<T, Allocator> (lhs.m_allocator, lhs.m_vaddr + n * sizeof(T));
}

template <typename T, typename Allocator>
constexpr Iterator<T, Allocator> operator+(size_t n, const Iterator<T, Allocator> &rhs)
{
    return Iterator<T, Allocator> (rhs.m_allocator, rhs.m_vaddr + n * sizeof(T));
}

template <typename T, typename Allocator>
constexpr Iterator<T, Allocator> operator-(const Iterator<T, Allocator> &lhs, size_t n)
{
    return Iterator<T, Allocator> (lhs.m_allocator, lhs.m_vaddr - n * sizeof(T));
}

template <typename T, typename Allocator>
constexpr Iterator<T, Allocator> operator-(size_t n, const Iterator<T, Allocator> &rhs)
{
    return Iterator<T, Allocator> (rhs.m_allocator, rhs.m_vaddr - n * sizeof(T));
}

#endif // ITERATOR_H
