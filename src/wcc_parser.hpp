#ifndef WCC_PARSER_HPP_
#define WCC_PARSER_HPP_

#include "./wcc_alexer.hpp"
#include "./wcc_memory.hpp"
#include "./wcc_types.hpp"

template <typename T>
struct Seq
{
    T unwrap;
    Seq<T> *next;
};

template <typename T>
void print1(FILE *stream, Seq<T> *seq)
{
    print(stream, "(");
    while (seq) {
        print(stream, seq->unwrap, " ");
    }
    print(stream, ")");
}

struct Var_Def
{
    String_View name;
    Type type;
};

enum class Statement_Kind
{
    Local_Var_Def,
    While,
    If,
    Assignment,
    Expression,
    Return,
};

enum class Expression_Kind
{
    Number_Literal,
    Variable,
    Plus,
    Greater,
    Type_Cast,
    Less_Equals,
    Minus,
    Rem,
    And,
    Multiply,
    Equals,
    Bool_Literal,
    Func_Call,
};

void print1(FILE *stream, Expression_Kind expression_kind);

const Token_Type binary_ops[] = {
    Token_Type::And,
    Token_Type::Equals_Equals,
    Token_Type::Less_Equals,
    Token_Type::Greater,
    Token_Type::Plus,
    Token_Type::Minus,
    Token_Type::Rem,
    Token_Type::Asterisk,
};
const size_t binary_ops_count = sizeof(binary_ops) / sizeof(binary_ops[0]);

const Expression_Kind binary_op_kinds[] = {
    Expression_Kind::And,
    Expression_Kind::Equals,
    Expression_Kind::Less_Equals,
    Expression_Kind::Greater,
    Expression_Kind::Plus,
    Expression_Kind::Minus,
    Expression_Kind::Rem,
    Expression_Kind::Multiply,
};
const size_t binary_op_kinds_count = sizeof(binary_op_kinds) / sizeof(binary_op_kinds[0]);

static_assert(binary_ops_count == binary_op_kinds_count);

struct Bool_Literal
{
    bool unwrap;
};

struct Number_Literal
{
    uint64_t unwrap;
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

struct Binary_Op
{
    Expression *lhs;
    Expression *rhs;
};

struct Func_Call
{
    String_View func_name;
    Seq<Expression*> *args;
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
        Binary_Op binary_op;
        Type_Cast type_cast;
        Bool_Literal bool_literal;
        Func_Call func_call;
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

struct If
{
    Expression *condition;
    Block *then;
    Block *elze;
};

struct Assignment
{
    String_View var_name;
    Expression *value;
};

struct Return
{
    Expression *value;
};

struct Statement
{
    Statement_Kind kind;
    Type type;
    size_t offset;
    union
    {
        Local_Var_Def local_var_def;
        While hwile;
        If iph;
        Assignment assignment;
        Expression *expression;
        Return reeturn;
    };
};

// TODO: Use Seq<T> for Block of Statements
struct Block
{
    Statement statement;
    Block *next;
};

// TODO: Use Seq<T> for Args_List
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

struct Include
{
    String_View module_name;
};

enum class Top_Def_Kind
{
    Func_Def,
    Include,
};

struct Top_Def
{
    Top_Def_Kind kind;
    union
    {
        Func_Def func_def;
        Include include;
    };
};

struct Module
{
    Seq<Top_Def> *top_defs;
};

// TODO: pretty-printing for AST
void print1(FILE *stream, Top_Def top_def);
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
    If parse_if();
    Type parse_type_annotation();
    Type parse_type();
    Var_Def parse_var_def();
    Args_List *parse_args_list();
    Func_Def parse_func_def();
    Statement parse_statement();
    Block *parse_block();
    Statement parse_dummy_statement();
    Assignment parse_subtract_assignment();
    Assignment parse_add_assignment();
    Expression *parse_binary_op(size_t binary_op_priority);
    Expression *parse_expression();
    Expression *parse_primary();
    Return parse_return();
    Include parse_include();
    Top_Def parse_top_def();
    Module parse_module();
};

#endif  // WCC_PARSER_HPP_
