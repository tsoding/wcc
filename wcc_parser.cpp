#include "./wcc_parser.hpp"
#include "./wcc_memory.hpp"

void print1(FILE *stream, Top_Level_Def *top_level_def)
{
    print(stream, "(");
    while (top_level_def) {
        print(stream, top_level_def->func_def, " ");
        top_level_def = top_level_def->next;
    }
    print(stream, ")");
}

void print1(FILE *stream, Module module)
{
    print(stream, "(module ", module.top_level_defs, ")");
}

void print1(FILE *stream, Expression expression)
{
    switch (expression.type) {
    case Expression_Type::Number_Literal:
        print(stream, expression.number_literal.unwrap);
        break;
    case Expression_Type::Variable:
        print(stream, expression.variable.name);
        break;
    case Expression_Type::Plus:
        print(stream, "(+ ", *expression.plus.lhs, " ", *expression.plus.rhs, ")");
        break;
    case Expression_Type::Greater:
        print(stream, "(> ", *expression.greater.lhs, " ", *expression.greater.rhs, ")");
        break;
    }
}

void print1(FILE *stream, Statement statement)
{
    switch (statement.type) {
    case Statement_Type::While:
        print(stream, "(while ", *statement.hwile.condition, " ", statement.hwile.body, ")");
        break;
    case Statement_Type::Local_Var_Def:
        print(stream, "(local ", statement.local_var_def.def, " ", *statement.local_var_def.value, ")");
        break;
    case Statement_Type::Assignment:
        print(stream, "(= ", statement.assignment.var_name, " ", *statement.assignment.value, ")");
        break;
    case Statement_Type::Subtract_Assignment:
        print(stream, "(-= ", statement.subtract_assignment.var_name, " ", *statement.subtract_assignment.value, ")");
        break;
    case Statement_Type::None:
        print(stream, "None");
        break;
    case Statement_Type::Expression:
        print(stream, *statement.expression);
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
    While result = {};

    expect_token_type(Token_Type::While);
    tokens.chop(1);

    expect_token_type(Token_Type::Open_Paren);
    tokens.chop(1);

    result.condition = parse_expression();

    expect_token_type(Token_Type::Closed_Paren);
    tokens.chop(1);

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

    assignment.value = parse_expression();

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

    subtract_assignment.value = parse_expression();

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
            result.type = Statement_Type::Expression;
            result.expression = parse_expression();
            expect_token_type(Token_Type::Semicolon);
            tokens.chop(1);
        }
        break;
    default:
        result.type = Statement_Type::Expression;
        result.expression = parse_expression();
        expect_token_type(Token_Type::Semicolon);
        tokens.chop(1);
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

    local_var_def.value = parse_expression();

    expect_token_type(Token_Type::Semicolon);
    tokens.chop(1);

    return local_var_def;
}

Expression *Parser::parse_primary()
{
    assert(tokens.count > 0);

    Expression *primary_expression = memory.alloc<Expression>();

    switch (tokens.items->type) {
    case Token_Type::Number_Literal: {
        primary_expression->type = Expression_Type::Number_Literal;
        Maybe<uint32_t> x = tokens.items->text.as_integer<uint32_t>();
        assert(x.has_value);
        primary_expression->number_literal.unwrap = x.unwrap;
        tokens.chop(1);
    } break;

    case Token_Type::Symbol: {
        primary_expression->type = Expression_Type::Variable;
        primary_expression->variable.name = tokens.items->text;
        tokens.chop(1);
    } break;

    case Token_Type::Open_Paren: {
        tokens.chop(1);
        primary_expression = parse_expression();
        expect_token_type(Token_Type::Closed_Paren);
        tokens.chop(1);
    } break;

    default:
        fail("Unexpected token in an expression");
    }

    return primary_expression;
}

Expression *Parser::parse_plus_expression()
{
    Expression *lhs = parse_primary();

    if (tokens.count == 0 || tokens.items->type != Token_Type::Plus) {
        return lhs;
    }

    expect_token_type(Token_Type::Plus);
    tokens.chop(1);

    Expression *rhs = parse_primary();

    Expression *plus_expression = memory.alloc<Expression>();
    plus_expression->type = Expression_Type::Plus;
    plus_expression->plus.lhs = lhs;
    plus_expression->plus.rhs = rhs;

    return plus_expression;
}

Expression *Parser::parse_greater_expression()
{
    Expression *lhs = parse_plus_expression();

    if (tokens.count == 0 || tokens.items->type != Token_Type::Greater) {
        return lhs;
    }

    expect_token_type(Token_Type::Greater);
    tokens.chop(1);

    Expression *rhs = parse_plus_expression();

    Expression *greater_expression = memory.alloc<Expression>();
    greater_expression->type = Expression_Type::Greater;
    greater_expression->greater.lhs = lhs;
    greater_expression->greater.rhs = rhs;

    return greater_expression;
}

Expression *Parser::parse_expression()
{
    assert(tokens.count > 0);

    return parse_greater_expression();
}

Top_Level_Def *Parser::parse_top_level_def()
{
    auto top_level_def = memory.alloc<Top_Level_Def>();
    top_level_def->func_def = parse_func_def();
    return top_level_def;
}

Module Parser::parse_module()
{
    Module module = {};

    Top_Level_Def *last_top_level_def = nullptr;

    while (tokens.count > 0) {
        auto top_level_def = parse_top_level_def();

        if (module.top_level_defs == nullptr) {
            module.top_level_defs = top_level_def;
        } else {
            last_top_level_def->next = top_level_def;
        }

        last_top_level_def = top_level_def;
    }

    return module;
}
