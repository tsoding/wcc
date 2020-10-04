#ifndef WCC_MEMORY_HPP_
#define WCC_MEMORY_HPP_

struct Memory
{
    size_t capacity;
    size_t size;
    uint8_t *buffer;

    template <typename T>
    T *alloc()
    {
        assert(size + sizeof(T) <= capacity);
        T *result = reinterpret_cast<T*>(buffer + size);
        memset(result, 0, sizeof(T));
        size += sizeof(T);
        return result;
    }
};

#endif  // WCC_MEMORY_HPP_
