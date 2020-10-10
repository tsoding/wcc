#include "./wcc_types.hpp"

Type_Kind kind_of_type(Type type)
{
    switch (type) {
    case Type::U0:
    case Type::U8:
    case Type::U32:
    case Type::U64:
        return Type_Kind::Unsigned_Integer;
    case Type::Unchecked:
        return Type_Kind::Unchecked;
    }

    assert(0 && "This can only happen if the memory is corrupted");
    return Type_Kind::Unchecked;
}

size_t size_of_type(Type type)
{
    switch (type) {
    case Type::U0:  return 0;
    case Type::U8:  return 1;
    case Type::U32: return 4;
    case Type::U64: return 8;
    case Type::Unchecked:
        assert(0 && "Impossible to get the size of unchecked type");
        return 0;
    }

    assert(0 && "This can only happen if the memory is corrupted");
    return 0;
}

bool is_kind_number(Type_Kind kind)
{
    switch (kind) {
    case Type_Kind::Unsigned_Integer:
    case Type_Kind::Signed_Integer:
    case Type_Kind::Floating_Point:
        return true;
    case Type_Kind::Unchecked:
        assert(0 && "Impossible to get the sort of unchecked type");
        return false;
    }

    assert(0 && "This can only happen if the memory is corrupted");
    return false;
}
