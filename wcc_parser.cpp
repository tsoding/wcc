#include "./wcc_parser.hpp"
#include "./wcc_memory.hpp"

void print1(FILE *stream, Expression expression)
{
    print(stream, "<expression>");
}

void print1(FILE *stream, Statement statement)
{
    switch (statement.type) {
    case Statement_Type::While:
        print(stream, "(while ", statement.hwile.condition, " ", statement.hwile.body, ")");
        break;
    case Statement_Type::Local_Var_Def:
        print(stream, "(local ", statement.local_var_def.def, " ", statement.local_var_def.value, ")");
        break;
    case Statement_Type::Assignment:
        print(stream, "(= ", statement.assignment.var_name, " ", statement.assignment.value, ")");
        break;
    case Statement_Type::Subtract_Assignment:
        print(stream, "(-= ", statement.subtract_assignment.var_name, " ", statement.subtract_assignment.value, ")");
        break;
    case Statement_Type::None:
        print(stream, "None");
        break;
    case Statement_Type::Expression:
        print(stream, statement.expression);
        break;
    }
}

void print1(FILE *stream, Block *block)
{
    print(stream, "(");
    while (block) {
        print(stream, block->statement, " ");
        block = block->next;
    }
    print(stream, ")");
}

void print1(FILE *stream, Type type)
{
    switch (type) {
    case Type::I32:
        print(stream, "Type::I32");
        break;
    }
}

void print1(FILE *stream, Var_Def var_def)
{
    print(stream, "(", var_def.name, " ", var_def.type, ")");
}

void print1(FILE *stream, Args_List *args_list)
{
    print(stream, "(");
    while (args_list) {
        print(stream, args_list->var_def, " ");
        args_list = args_list->next;
    }
    print(stream, ")");
}

void print1(FILE *stream, Func_Def func_def)
{
    print(stream, "(func_def ", func_def.name, " ", func_def.args_list, " ",
          func_def.return_type, " ",
          func_def.body, ")");
}

Var_Def Parser::parse_var_def()
{
    Var_Def var_def = {};

    expect_token_type(Token_Type::Symbol);
    var_def.name = tokens.items->text;
    tokens.chop(1);

    var_def.type = parse_type_annotation();

    return var_def;
}

Args_List *Parser::parse_args_list()
{
    expect_token_type(Token_Type::Open_Paren);
    tokens.chop(1);

    Args_List *result = nullptr;
    Args_List *last_arg = nullptr;

    while (tokens.count > 0) {
        Args_List *arg = memory.alloc<Args_List>();
        arg->var_def = parse_var_def();

        if (last_arg == nullptr)  {
            result = arg;
        } else {
            last_arg->next = arg;
        }
        last_arg = arg;

        if (tokens.items->type != Token_Type::Comma) {
            expect_token_type(Token_Type::Closed_Paren);
            tokens.chop(1);
            return result;
        }

        tokens.chop(1);
    }

    return result;
}

Type Parser::parse_type_annotation()
{
    expect_token_type(Token_Type::Colon);
    tokens.chop(1);

    expect_token_type(Token_Type::Symbol);
    auto type_name = tokens.items->text;

    if (type_name != "i32"_sv) {
        fail("Unknown type `", type_name, "`");
    }
    tokens.chop(1);
    return Type::I32;
}

While Parser::parse_while()
{
    expect_token_type(Token_Type::While);
    tokens.chop(1);

    expect_token_type(Token_Type::Open_Paren);
    tokens.chop(1);
    while (tokens.count > 0 && tokens.items->type != Token_Type::Closed_Paren) {
        tokens.chop(1);
    }
    expect_token_type(Token_Type::Closed_Paren);
    tokens.chop(1);

    While result = {};
    result.body = parse_block();

    return result;
}

Statement Parser::parse_dummy_statement()
{
    while (tokens.count > 0 && tokens.items->type != Token_Type::Semicolon) {
        tokens.chop(1);
    }

    expect_token_type(Token_Type::Semicolon);
    tokens.chop(1);

    return {};
}

Assignment Parser::parse_assignment()
{
    Assignment assignment = {};

    expect_token_type(Token_Type::Symbol);
    assignment.var_name = tokens.items->text;
    tokens.chop(1);

    expect_token_type(Token_Type::Equals);
    tokens.chop(1);

    auto expression_parser = *this;
    expression_parser.tokens = tokens.chop_until(is_semicolon);
    assignment.value = expression_parser.parse_expression();

    expect_token_type(Token_Type::Semicolon);
    tokens.chop(1);

    return assignment;
}

Subtract_Assignment Parser::parse_subtract_assignment()
{
    Subtract_Assignment subtract_assignment = {};

    expect_token_type(Token_Type::Symbol);
    subtract_assignment.var_name = tokens.items->text;
    tokens.chop(1);

    expect_token_type(Token_Type::Minus_Equals);
    tokens.chop(1);

    auto expression_parser = *this;
    expression_parser.tokens = tokens.chop_until(is_semicolon);
    subtract_assignment.value = expression_parser.parse_expression();

    expect_token_type(Token_Type::Semicolon);
    tokens.chop(1);

    return subtract_assignment;
}

Statement Parser::parse_statement()
{

    Statement result = {};

    assert(tokens.count > 0);
    switch (tokens.items->type) {
    case Token_Type::While:
        result.type = Statement_Type::While;
        result.hwile = parse_while();
        break;
    case Token_Type::Local:
        result.type = Statement_Type::Local_Var_Def;
        result.local_var_def = parse_local_var_def();
        break;
    case Token_Type::Symbol:
        assert(tokens.count > 1);
        switch (tokens.items[1].type) {
        case Token_Type::Equals:
            result.type = Statement_Type::Assignment;
            result.assignment = parse_assignment();
            break;
        case Token_Type::Minus_Equals:
            result.type = Statement_Type::Subtract_Assignment;
            result.subtract_assignment = parse_subtract_assignment();
            break;
        default:
            result = parse_dummy_statement();
        }
        break;
    default:
        result = parse_dummy_statement();
    }

    return result;
}

Block *Parser::parse_block()
{
    expect_token_type(Token_Type::Open_Curly);
    tokens.chop(1);

    Block *result = nullptr;
    Block *last_st = nullptr;

    while (tokens.count > 0 && tokens.items->type != Token_Type::Closed_Curly) {
        Block *st = memory.alloc<Block>();
        st->statement = parse_statement();

        if (last_st == nullptr)  {
            result = st;
        } else {
            last_st->next = st;
        }
        last_st = st;
    }

    expect_token_type(Token_Type::Closed_Curly);
    tokens.chop(1);

    return result;
}

Func_Def Parser::parse_func_def()
{
    Func_Def func_def = {};

    expect_token_type(Token_Type::Func);
    tokens.chop(1);

    expect_token_type(Token_Type::Symbol);
    func_def.name = tokens.items->text;
    tokens.chop(1);

    func_def.args_list = parse_args_list();
    func_def.return_type = parse_type_annotation();
    func_def.body = parse_block();

    return func_def;
}

Local_Var_Def Parser::parse_local_var_def()
{
    Local_Var_Def local_var_def = {};

    expect_token_type(Token_Type::Local);
    tokens.chop(1);

    local_var_def.def = parse_var_def();

    expect_token_type(Token_Type::Equals);
    tokens.chop(1);

    auto expression_parser = *this;
    expression_parser.tokens = tokens.chop_until(is_semicolon);
    local_var_def.value = expression_parser.parse_expression();

    expect_token_type(Token_Type::Semicolon);
    tokens.chop(1);

    return local_var_def;
}

Expression Parser::parse_expression()
{
    return {};
}
