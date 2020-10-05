#ifndef WCC_MEMORY_HPP_
#define WCC_MEMORY_HPP_

struct Linear_Memory
{
    size_t capacity;
    size_t size;
    uint8_t *buffer;

    template <typename T>
    T *top()
    {
        return reinterpret_cast<T*>(buffer + size);
    }

    template <typename T>
    T *alloc(size_t n = 1)
    {
        assert(n > 0);
        assert(size + sizeof(T) * n <= capacity);
        T *result = top<T>();
        memset(result, 0, sizeof(T) * n);
        size += sizeof(T) * n;
        return result;
    }
};

#endif  // WCC_MEMORY_HPP_
