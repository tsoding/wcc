#ifndef WCC_TYPES_HPP_
#define WCC_TYPES_HPP_

enum class Type
{
    Unchecked,
    U0,
    U8,
    U32,
    U64,
};

size_t size_of_type(Type type);

#endif  // WCC_TYPES_HPP_
