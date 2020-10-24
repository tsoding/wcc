#include "./wcc_parser.hpp"
#include "./wcc_memory.hpp"

void print1(FILE *stream, Top_Def top_def)
{
    print(stream, top_def.func_def);
}

void print1(FILE *stream, Module module)
{
    print(stream, "(module ", module.top_defs, ")");
}

void print1(FILE *stream, Expression_Kind expression_kind)
{
    switch (expression_kind) {
    case Expression_Kind::Number_Literal:
        print(stream, "Number_Literal");
        break;
    case Expression_Kind::Variable:
        print(stream, "Variable");
        break;
    case Expression_Kind::Plus:
        print(stream, "Plus");
        break;
    case Expression_Kind::Greater:
        print(stream, "Greater");
        break;
    case Expression_Kind::Type_Cast:
        print(stream, "Type_Cast");
        break;
    case Expression_Kind::Less_Equals:
        print(stream, "Less_Equals");
        break;
    case Expression_Kind::Minus:
        print(stream, "Minus");
        break;
    case Expression_Kind::Rem:
        print(stream, "Rem");
        break;
    case Expression_Kind::And:
        print(stream, "And");
        break;
    case Expression_Kind::Multiply:
        print(stream, "Multiply");
        break;
    case Expression_Kind::Equals:
        print(stream, "Equals");
        break;
    case Expression_Kind::Bool_Literal:
        print(stream, "Bool_Literal");
        break;
    }
}

void print1(FILE *stream, Expression expression)
{
    switch (expression.kind) {
    case Expression_Kind::Type_Cast:
        print(stream, "(cast ", expression.type_cast.type, " ", *expression.type_cast.expression, ")");
        break;
    case Expression_Kind::Number_Literal:
        print(stream, expression.number_literal.unwrap);
        break;
    case Expression_Kind::Bool_Literal:
        if (expression.bool_literal.unwrap) {
            print(stream, "true");
        } else {
            print(stream, "false");
        }
        break;
    case Expression_Kind::Variable:
        print(stream, expression.variable.name);
        break;
    case Expression_Kind::Plus:
        print(stream, "(+ ", *expression.binary_op.lhs, " ", *expression.binary_op.rhs, ")");
        break;
    case Expression_Kind::Multiply:
        print(stream, "(* ", *expression.binary_op.lhs, " ", *expression.binary_op.rhs, ")");
        break;
    case Expression_Kind::Minus:
        print(stream, "(- ", *expression.binary_op.lhs, " ", *expression.binary_op.rhs, ")");
        break;
    case Expression_Kind::Rem:
        print(stream, "(% ", *expression.binary_op.lhs, " ", *expression.binary_op.rhs, ")");
        break;
    case Expression_Kind::And:
        print(stream, "(&& ", *expression.binary_op.lhs, " ", *expression.binary_op.rhs, ")");
        break;
    case Expression_Kind::Greater:
        print(stream, "(> ", *expression.binary_op.lhs, " ", *expression.binary_op.rhs, ")");
        break;
    case Expression_Kind::Less_Equals:
        print(stream, "(<= ", *expression.binary_op.lhs, " ", *expression.binary_op.rhs, ")");
        break;
    case Expression_Kind::Equals:
        print(stream, "(== ", *expression.binary_op.lhs, " ", *expression.binary_op.rhs, ")");
    }
}

void print1(FILE *stream, Statement statement)
{
    switch (statement.kind) {
    case Statement_Kind::If:
        print(stream, "(if ", *statement.iph.condition, " ", statement.iph.then, " ", statement.iph.elze, ")");
        break;
    case Statement_Kind::While:
        print(stream, "(while ", *statement.hwile.condition, " ", statement.hwile.body, ")");
        break;
    case Statement_Kind::Local_Var_Def:
        print(stream, "(local ", statement.local_var_def.def, " ", *statement.local_var_def.value, ")");
        break;
    case Statement_Kind::Assignment:
        print(stream, "(= ", statement.assignment.var_name, " ", *statement.assignment.value, ")");
        break;
    case Statement_Kind::Return:
        print(stream, "(return ", *statement.reeturn.value, ")");
        break;
    case Statement_Kind::Expression:
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
    case Type::Unchecked:
        print(stream, "Type::Unchecked");
        break;
    case Type::U0:
        print(stream, "u0");
        break;
    case Type::U8:
        print(stream, "u8");
        break;
    case Type::U32:
        print(stream, "u32");
        break;
    case Type::U64:
        print(stream, "u64");
        break;
    case Type::Bool:
        print(stream, "bool");
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
        Args_List *arg = memory->alloc<Args_List>();
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

    if (type_name == "u32"_sv) {
        tokens.chop(1);
        return Type::U32;
    } else if (type_name == "u64"_sv) {
        tokens.chop(1);
        return Type::U64;
    } else if (type_name == "u8"_sv) {
        tokens.chop(1);
        return Type::U8;
    } else if (type_name == "bool"_sv) {
        tokens.chop(1);
        return Type::Bool;
    } else {
        reporter.fail(current_offset(), "Unknown type `", type_name, "`");
        return Type::Unchecked;
    }
}

If Parser::parse_if()
{
    If result = {};
    expect_token_type(Token_Type::If);
    tokens.chop(1);

    expect_token_type(Token_Type::Open_Paren);
    tokens.chop(1);

    result.condition = parse_expression();

    expect_token_type(Token_Type::Closed_Paren);
    tokens.chop(1);

    result.then = parse_block();

    if (tokens.count > 0 && tokens.items->type == Token_Type::Else) {
        tokens.chop(1);
        result.elze = parse_block();
    }

    return result;
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

Assignment Parser::parse_add_assignment()
{
    Assignment assignment = {};

    expect_token_type(Token_Type::Symbol);
    assignment.var_name = tokens.items->text;
    tokens.chop(1);

    expect_token_type(Token_Type::Plus_Equals);
    tokens.chop(1);

    assignment.value = memory->alloc<Expression>();
    assignment.value->kind = Expression_Kind::Plus;
    assignment.value->binary_op.lhs = memory->alloc<Expression>();
    assignment.value->binary_op.lhs->kind = Expression_Kind::Variable;
    assignment.value->binary_op.lhs->variable.name = assignment.var_name;
    assignment.value->binary_op.rhs = parse_expression();

    expect_token_type(Token_Type::Semicolon);
    tokens.chop(1);

    return assignment;
}

Assignment Parser::parse_subtract_assignment()
{
    Assignment assignment = {};

    expect_token_type(Token_Type::Symbol);
    assignment.var_name = tokens.items->text;
    tokens.chop(1);

    expect_token_type(Token_Type::Minus_Equals);
    tokens.chop(1);

    assignment.value = memory->alloc<Expression>();
    assignment.value->kind = Expression_Kind::Minus;
    assignment.value->binary_op.lhs = memory->alloc<Expression>();
    assignment.value->binary_op.lhs->kind = Expression_Kind::Variable;
    assignment.value->binary_op.lhs->variable.name = assignment.var_name;
    assignment.value->binary_op.rhs = parse_expression();

    expect_token_type(Token_Type::Semicolon);
    tokens.chop(1);

    return assignment;
}

Return Parser::parse_return()
{
    expect_token_type(Token_Type::Return);
    tokens.chop(1);

    Return reeturn = {};
    reeturn.value = parse_expression();

    expect_token_type(Token_Type::Semicolon);
    tokens.chop(1);

    return reeturn;
}

Statement Parser::parse_statement()
{
    Statement result = {};

    assert(tokens.count > 0);
    result.offset = current_offset();

    switch (tokens.items->type) {
    case Token_Type::While:
        result.kind = Statement_Kind::While;
        result.hwile = parse_while();
        break;
    case Token_Type::If:
        result.kind = Statement_Kind::If;
        result.iph = parse_if();
        break;
    case Token_Type::Local:
        result.kind = Statement_Kind::Local_Var_Def;
        result.local_var_def = parse_local_var_def();
        break;
    case Token_Type::Symbol:
        assert(tokens.count > 1);
        switch (tokens.items[1].type) {
        case Token_Type::Equals:
            result.kind = Statement_Kind::Assignment;
            result.assignment = parse_assignment();
            break;
        case Token_Type::Minus_Equals:
            result.kind = Statement_Kind::Assignment;
            result.assignment = parse_subtract_assignment();
            break;
        case Token_Type::Plus_Equals:
            result.kind = Statement_Kind::Assignment;
            result.assignment = parse_add_assignment();
            break;
        default:
            result.kind = Statement_Kind::Expression;
            result.expression = parse_expression();
            expect_token_type(Token_Type::Semicolon);
            tokens.chop(1);
        }
        break;
    case Token_Type::Return:
        result.kind = Statement_Kind::Return;
        result.reeturn = parse_return();
        break;
    default:
        result.kind = Statement_Kind::Expression;
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
        Block *st = memory->alloc<Block>();
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
    func_def.offset = current_offset();
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

    Expression *primary_expression = memory->alloc<Expression>();
    primary_expression->offset = current_offset();

    switch (tokens.items->type) {
    case Token_Type::True: {
        primary_expression->kind = Expression_Kind::Bool_Literal;
        primary_expression->bool_literal = {true};
        tokens.chop(1);
    } break;

    case Token_Type::False: {
        primary_expression->kind = Expression_Kind::Bool_Literal;
        primary_expression->bool_literal = {false};
        tokens.chop(1);
    } break;

    case Token_Type::Number_Literal: {
        primary_expression->kind = Expression_Kind::Number_Literal;
        auto x = tokens.items->text.as_integer<uint32_t>();
        if (!x.has_value) {
            reporter.fail(current_offset(), "`", tokens.items->text, "` is not a number");
        }
        primary_expression->number_literal.unwrap = x.unwrap;
        tokens.chop(1);
    } break;

    case Token_Type::Symbol: {
        primary_expression->kind = Expression_Kind::Variable;
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
        reporter.fail(current_offset(), "Unexpected token in an expression `", tokens.items->type, "`");
    }

    return primary_expression;
}

Expression *Parser::parse_binary_op(size_t binary_op_priority)
{
    if (binary_op_priority < binary_ops_count) {
        Expression *lhs = parse_binary_op(binary_op_priority + 1);

        if (tokens.count == 0 || tokens.items->type != binary_ops[binary_op_priority]) {
            return lhs;
        }

        expect_token_type(binary_ops[binary_op_priority]);
        tokens.chop(1);

        Expression *rhs = parse_binary_op(binary_op_priority + 1);

        Expression *op = memory->alloc<Expression>();
        op->offset = lhs->offset;
        op->kind = binary_op_kinds[binary_op_priority];
        op->binary_op.lhs = lhs;
        op->binary_op.rhs = rhs;
        return op;
    } else {
        return parse_primary();
    }
}

Expression *Parser::parse_expression()
{
    assert(tokens.count > 0);

    return parse_binary_op(0);
}

Top_Def Parser::parse_top_def()
{
    Top_Def top_def = {};
    top_def.func_def = parse_func_def();
    return top_def;
}

Module Parser::parse_module()
{
    Module module = {};

    Seq<Top_Def> *last_top_def = nullptr;

    while (tokens.count > 0) {
        auto top_def = memory->alloc<Seq<Top_Def>>();
        top_def->unwrap = parse_top_def();

        if (module.top_defs == nullptr) {
            module.top_defs = top_def;
        } else {
            last_top_def->next = top_def;
        }

        last_top_def = top_def;
    }

    return module;
}
