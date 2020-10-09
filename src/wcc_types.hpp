#ifndef WCC_TYPES_HPP_
#define WCC_TYPES_HPP_

enum class Type
{
    Unchecked,
    U0,
    U32,
    U64,
};

enum class Type_Kind
{
    Unchecked,
    Unsigned_Integer,
    Signed_Integer,
    Floating_Point,
};

size_t size_of_type(Type type);
Type_Kind kind_of_type(Type type);
bool is_kind_number(Type_Kind kind);

#endif  // WCC_TYPES_HPP_
