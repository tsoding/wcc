#ifndef WCC_PARSER_HPP_
#define WCC_PARSER_HPP_

template <typename T>
struct Buffer
{
    T *items;
    size_t count;
};

enum class Type
{
    I32
};

struct Var_Def
{
    String_View name;
    Type type;
};

enum class Statement_Type
{
    Local_Var_Def,
    While,
    Assignment,
    Subtract_Assignment,
    Expression,
};

enum class Expression_Type
{
    Number_Literal,
    Variable,
    Plus,
    Greater,
};

struct Number_Literal
{
    uint32_t unwrap;
};

struct Variable
{
    String_View name;
};

struct Expression;

struct Plus
{
    Expression *lhs;
    Expression *rhs;
};

struct Greater
{
    Expression *lhs;
    Expression *rhs;
};

struct Expression
{
    Expression_Type type;
    union
    {
        Number_Literal number_literal;
        Variable variable;
        Plus plus;
        Greater greater;
    };
};

struct Block;

struct Local_Var_Def
{
    Var_Def def;
    Expression value;
};

struct While
{
    Expression condition;
    Block *body;
};

struct Assignment
{
    String_View var_name;
    Expression value;
};

struct Subtract_Assignment
{
    String_View var_name;
    Expression value;
};

struct Statement
{
    Statement_Type type;
    union
    {
        Local_Var_Def local_var_def;
        While hwile;
        Assignment assignment;
        Subtract_Assignment subtract_assignment;
        Expression expression;
    };
};

struct Block
{
    Buffer<Statement> statements;
};

struct Func_Def
{
    String_View name;
    Buffer<Var_Def> args_list;
    Type return_type;
    Block body;
};

#endif  // WCC_PARSER_HPP_
