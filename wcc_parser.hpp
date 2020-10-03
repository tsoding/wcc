#ifndef WCC_PARSER_HPP_
#define WCC_PARSER_HPP_

#include "./wcc_alexer.hpp"
#include "./wcc_memory.hpp"

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
    None,
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
};

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
};

struct Escape
{
    String_View unwrap;
};

void print1(FILE *stream, Escape escape)
{
    for (size_t i = 0; i < escape.unwrap.count; ++i) {
        switch (escape.unwrap.data[i]) {
        case '\a': print(stream, "\\a"); break;
        case '\b': print(stream, "\\b"); break;
        case '\f': print(stream, "\\f"); break;
        case '\n': print(stream, "\\n"); break;
        case '\r': print(stream, "\\r"); break;
        case '\t': print(stream, "\\t"); break;
        case '\v': print(stream, "\\v"); break;
        default: print(stream, escape.unwrap.data[i]);
        }
    }
}

struct Parser
{
    Memory memory;
    View<Token> tokens;
    String_View input;
    String_View filename;

    template <typename... Args>
    void warn(Args... args)
    {
        assert(tokens.count > 0);
        Token place = *tokens.items;

        String_View input0 = input;
        String_View line = input0.chop_by_delim('\n');

        size_t offset = place.text.data - input.data;

        for (size_t line_number = 1; input0.count > 0; ++line_number) {
            line = input0.chop_by_delim('\n');

            if (offset <= line.count) {
                println(stderr, filename, ":", line_number, ":", offset, ": ", args...);
                break;
            }

            offset -= line.count + 1;
        }
    }

    template <typename... Args>
    void fail(Args... args)
    {
        warn(args...);

        println(stdout, "Unparsed tokens: ");
        for (size_t i = 0; i < tokens.count; ++i) {
            println(stdout, "  ", tokens.items[i].type, " -> \"", Escape { tokens.items[i].text }, "\"");
        }
        exit(1);
    }

    void expect_token_type(Token_Type expected_type)
    {
        assert(tokens.count > 0);
        if (expected_type != tokens.items->type) {
            fail("Expected ", expected_type, " but got `", Escape { tokens.items->text }, "`");
        }
    }

    Local_Var_Def parse_local_var_def();
    While parse_while();
    Type parse_type_annotation();
    Var_Def parse_var_def();
    Args_List *parse_args_list();
    Func_Def parse_func_def();
    Statement parse_statement();
    Block *parse_block();
    Statement parse_dummy_statement();
};

#endif  // WCC_PARSER_HPP_
