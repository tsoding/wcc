#ifndef WCC_TYPES_HPP_
#define WCC_TYPES_HPP_

enum class Type_Kind
{
    Unchecked,
    Primitive_Type,
    Pointer,
};

enum class Primitive_Type
{
    U0,
    U8,
    U32,
    U64,
    Bool,
};

struct Type;

struct Pointer
{
    Type *type;
};

struct Type
{
    Type_Kind kind;
    union
    {
        Primitive_Type primitive_type;
        Pointer pointer;
    };

    bool operator==(Type that)
    {
        if (this->kind != that.kind) {
            return false;
        }

        switch (this->kind) {
        case Type_Kind::Unchecked:
            return true;
        case Type_Kind::Primitive_Type:
            return this->primitive_type == that.primitive_type;
        case Type_Kind::Pointer:
            assert(this->pointer.type);
            assert(that.pointer.type);
            return *this->pointer.type == *that.pointer.type;
        }

        assert(0 && "Memory corruption?");
        return false;
    }

    bool operator!=(Type that)
    {
        return !(*this == that);
    }
};

constexpr Type primitive_type(Primitive_Type p)
{
    Type result = {};
    result.kind = Type_Kind::Primitive_Type;
    result.primitive_type = p;
    return result;
}

constexpr Type u0_type()
{
    return primitive_type(Primitive_Type::U0);
}

constexpr Type u8_type()
{
    return primitive_type(Primitive_Type::U8);
}

constexpr Type u32_type()
{
    return primitive_type(Primitive_Type::U32);
}

constexpr Type u64_type()
{
    return primitive_type(Primitive_Type::U64);
}

constexpr Type bool_type()
{
    return primitive_type(Primitive_Type::Bool);
}

#endif  // WCC_TYPES_HPP_
