#ifndef WCC_PARSER_HPP_
#define WCC_PARSER_HPP_

#include "./wcc_alexer.hpp"
#include "./wcc_memory.hpp"
#include "./wcc_types.hpp"

struct Var_Def
{
    String_View name;
    Type type;
};

enum class Statement_Kind
{
    Local_Var_Def,
    While,
    Assignment,
    Subtract_Assignment,
    Expression,
};

enum class Expression_Kind
{
    Number_Literal,
    Variable,
    Plus,
    Greater,
    Type_Cast,
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

struct Type_Cast
{
    Type type;
    Expression *expression;
};

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
    Expression_Kind kind;
    Type type;
    size_t offset;
    union
    {
        Number_Literal number_literal;
        Variable variable;
        Plus plus;
        Greater greater;
        Type_Cast type_cast;
    };
};

struct Block;

struct Local_Var_Def
{
    Var_Def def;
    Expression *value;
};

struct While
{
    Expression *condition;
    Block *body;
};

struct Assignment
{
    String_View var_name;
    Expression *value;
};

struct Subtract_Assignment
{
    String_View var_name;
    Expression *value;
};

struct Statement
{
    Statement_Kind kind;
    size_t offset;
    union
    {
        Local_Var_Def local_var_def;
        While hwile;
        Assignment assignment;
        Subtract_Assignment subtract_assignment;
        Expression *expression;
    };
};

struct Block
{
    Statement statement;
    Block *next;
};

struct Args_List
{
    Var_Def var_def;
    Args_List *next;
};

struct Func_Def
{
    String_View name;
    Args_List *args_list;
    Type return_type;
    Block *body;
    size_t offset;
};

struct Top_Level_Def
{
    Func_Def func_def;
    Top_Level_Def *next;
};

struct Module
{
    Top_Level_Def *top_level_defs;
};

// TODO: pretty-printing for AST
void print1(FILE *stream, Top_Level_Def top_level_def);
void print1(FILE *stream, Module module);
void print1(FILE *stream, Statement statement);
void print1(FILE *stream, Block *block);
void print1(FILE *stream, Func_Def func_def);
void print1(FILE *stream, Args_List *args_list);
void print1(FILE *stream, Var_Def var_def);
void print1(FILE *stream, Type type);

template <typename T>
struct View
{
    size_t count;
    T *items;

    void chop(size_t n)
    {
        if (n > count) {
            items += count;
            count = 0;
        } else {
            items  += n;
            count -= n;
        }
    }

    View<T> subview(size_t start, size_t count) const
    {
        if (start + count <= this->count) {
            return {count, items + start};
        }

        return {};
    }

    template <typename Predicate>
    View<T> chop_while(Predicate predicate)
    {
        size_t n = 0;
        while (n < count && predicate(items[n])) {
            n += 1;
        }

        View<T> chunk = subview(0, n);
        chop(n);

        return chunk;
    }

    template <typename Predicate>
    View<T> chop_until(Predicate predicate)
    {
        size_t n = 0;
        while (n < count && !predicate(items[n])) {
            n += 1;
        }

        View<T> chunk = subview(0, n);
        chop(n);

        return chunk;
    }
};

struct Parser
{
    Linear_Memory *memory;
    View<Token> tokens;
    Reporter reporter;

    size_t current_offset() const
    {
        assert(tokens.count > 0);
        return reporter.offset_from_input(tokens.items->text.data);
    }

    void dump_tokens()
    {
        for (size_t i = 0; i < tokens.count; ++i) {
            println(stdout, "  ", tokens.items[i].type, " -> \"", Escape { tokens.items[i].text }, "\"");
        }
    }

    void expect_token_type(Token_Type expected_type)
    {
        assert(tokens.count > 0);
        if (expected_type != tokens.items->type) {
            reporter.fail(current_offset(), "Expected ", expected_type, " but got `", Escape { tokens.items->text }, "`");
        }
    }

    Assignment parse_assignment();
    Local_Var_Def parse_local_var_def();
    While parse_while();
    Type parse_type_annotation();
    Var_Def parse_var_def();
    Args_List *parse_args_list();
    Func_Def parse_func_def();
    Statement parse_statement();
    Block *parse_block();
    Statement parse_dummy_statement();
    Subtract_Assignment parse_subtract_assignment();
    Expression *parse_plus_expression();
    Expression *parse_greater_expression();
    Expression *parse_expression();
    Expression *parse_primary();
    Top_Level_Def *parse_top_level_def();
    Module parse_module();
};

#endif  // WCC_PARSER_HPP_
