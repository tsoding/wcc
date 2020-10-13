#include "./wcc_types.hpp"

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
