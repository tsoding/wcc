#include "./wcc_type_checker.hpp"

Type Type_Checker::type_of_name(size_t offset, String_View name) const
{
    auto scope0 = scope;

    while (scope0) {
        Args_List *args_list = scope0->args_list;
        while (args_list) {
            if (name == args_list->var_def.name) {
                return args_list->var_def.type;
            }
            args_list = args_list->next;
        }
        scope0 = scope0->next;
    }

    reporter.fail(offset, "Could not find name `", name, "` in the current scope");
    return Type::Unchecked;
}

void Type_Checker::push_scope(Args_List *args_list)
{
    auto scope0 = memory->alloc<Scope>();
    scope0->args_list = args_list;
    scope0->next = scope;
    scope = scope0;
}

void Type_Checker::pop_scope()
{
    assert(scope);
    scope = scope->next;
}

void Type_Checker::push_var_def(Var_Def var_def)
{
    assert(scope);
    auto args_list = memory->alloc<Args_List>();
    args_list->var_def = var_def;
    args_list->next = scope->args_list;
    scope->args_list = args_list;
}

Type Type_Checker::check_types(size_t offset, Type expected_type, Type actual_type)
{
    if (expected_type != actual_type) {
        reporter.fail(offset, "Expected type `", expected_type, "` but got `", actual_type, "`");
    }

    return expected_type;
}

Expression *Type_Checker::cast_expression_to(Expression *expression, Type type)
{
    Expression *cast_expression = memory->alloc<Expression>();
    cast_expression->kind = Expression_Kind::Type_Cast;
    cast_expression->type = type;
    cast_expression->offset = expression->offset;
    cast_expression->type_cast.type = cast_expression->type;
    cast_expression->type_cast.expression = expression;
    return cast_expression;
}

bool is_expression_castable_to(Expression *expression, Type cast_type)
{
    switch (expression->type) {
    case Type::U0:
        switch (cast_type) {
        case Type::U0:
            return true;
        case Type::U8:
        case Type::U32:
        case Type::U64:
            return false;
        case Type::Unchecked:
            assert(0 && "Unchecked type in a checked context");
            break;
        }
        break;

    case Type::U8:
        switch (cast_type) {
        case Type::U0:
            return false;
        case Type::U8:
        case Type::U32:
        case Type::U64:
            return true;
        case Type::Unchecked:
            assert(0 && "Unchecked type in a checked context");
            break;
        }
        break;
    case Type::U32:
        switch (cast_type) {
        case Type::U0:
            return false;
        case Type::U8:
            return expression->kind == Expression_Kind::Number_Literal && expression->number_literal.unwrap <= 0xFF;
        case Type::U32:
        case Type::U64:
            return true;
        case Type::Unchecked:
            assert(0 && "Unchecked type in a checked context");
            break;
        }
        break;
    case Type::U64:
        switch (cast_type) {
        case Type::U0:
            return false;
        case Type::U8:
            return expression->kind == Expression_Kind::Number_Literal && expression->number_literal.unwrap <= 0xFF;
        case Type::U32:
            return expression->kind == Expression_Kind::Number_Literal && expression->number_literal.unwrap <= 0xFF'FF'FF'FF;
        case Type::U64:
            return true;
        case Type::Unchecked:
            assert(0 && "Unchecked type in a checked context");
            break;
        }
        break;

    case Type::Unchecked:
        assert(0 && "Unchecked type in a checked context");
        break;
    }

    assert(0 && "Memory corruption?");
    return false;
}

Expression *Type_Checker::try_implicitly_cast_expression_to(Expression *expression, Type cast_type)
{
    if (is_expression_castable_to(expression, cast_type)) {
        return cast_expression_to(expression, cast_type);
    } else {
        reporter.fail(expression->offset, "Expression of type `", expression->type, "` is not convertable to `", cast_type, "`");
    }

    return expression;
}

Type Type_Checker::check_types_of_local_var_def(Local_Var_Def *local_var_def)
{
    check_types_of_expression(local_var_def->value);
    local_var_def->value = try_implicitly_cast_expression_to(
        local_var_def->value,
        local_var_def->def.type);
    push_var_def(local_var_def->def);
    return local_var_def->def.type;
}

Type Type_Checker::check_types_of_while(While *hwile)
{
    // TODO: support for booleans (grep for @bool)
    check_types(hwile->condition->offset, Type::U32, check_types_of_expression(hwile->condition));
    check_types_of_block(hwile->body);
    return Type::U0;
}

Type Type_Checker::check_types_of_if(If *iph)
{
    // @bool
    check_types(iph->condition->offset, Type::U32, check_types_of_expression(iph->condition));
    auto then_type = check_types_of_block(iph->then);
    auto else_type = check_types_of_block(iph->elze);
    // TODO: unmatching then and else types should have offset of the last (or relevant) statement in a branch
    check_types(iph->condition->offset, then_type, else_type);
    return then_type;
}

Type Type_Checker::check_types_of_assignment(Assignment *assignment)
{
    // TODO: check_types_of_assignment should report errors on the assignment's offset, not its value one
    // TODO: assigment could be an expression that has the type of the variable it assigns
    size_t offset = assignment->value->offset;
    check_types(
        offset,
        type_of_name(offset, assignment->var_name),
        check_types_of_expression(assignment->value));
    return Type::U0;
}

Type Type_Checker::check_types_of_subtract_assignment(Subtract_Assignment *subtract_assignment)
{
    // @subass-sugar
    size_t offset = subtract_assignment->value->offset;
    check_types_of_expression(subtract_assignment->value);
    subtract_assignment->value = try_implicitly_cast_expression_to(
        subtract_assignment->value,
        type_of_name(offset, subtract_assignment->var_name));
    return Type::U0;
}

Type Type_Checker::check_types_of_expression(Expression *expression)
{
    assert(expression->type == Type::Unchecked);

    switch (expression->kind) {
    case Expression_Kind::Type_Cast: {
        check_types_of_expression(expression->type_cast.expression);
        expression->type = expression->type_cast.type;
    } break;

    case Expression_Kind::Number_Literal: {
        if (expression->number_literal.unwrap <= 0xFF) {
            expression->type = Type::U8;
        } else if (expression->number_literal.unwrap <= 0xFF'FF'FF'FF) {
            expression->type = Type::U32;
        } else {
            expression->type = Type::U64;
        }
    } break;

    case Expression_Kind::Variable: {
        expression->type = type_of_name(expression->offset, expression->variable.name);
    } break;

    case Expression_Kind::Multiply:
    case Expression_Kind::Rem:
    case Expression_Kind::Minus:
    case Expression_Kind::Plus: {
        Type lhs_type = check_types_of_expression(expression->binary_op.lhs);
        Type rhs_type = check_types_of_expression(expression->binary_op.rhs);

        expression->type = check_types(expression->binary_op.rhs->offset, lhs_type, rhs_type);
    } break;

    case Expression_Kind::Less_Equals:
    case Expression_Kind::Greater: {
        Type lhs_type = check_types_of_expression(expression->binary_op.lhs);
        Type rhs_type = check_types_of_expression(expression->binary_op.rhs);

        if (is_expression_castable_to(expression->binary_op.lhs, rhs_type)) {
            expression->binary_op.lhs = cast_expression_to(expression->binary_op.lhs, rhs_type);
        } else if (is_expression_castable_to(expression->binary_op.lhs, lhs_type)) {
            expression->binary_op.rhs = try_implicitly_cast_expression_to(expression->binary_op.rhs, lhs_type);
        } else {
            reporter.fail(expression->binary_op.rhs->offset, "Types `", lhs_type, "` and `", rhs_type, "` are not comparable with each other");
        }

        expression->type = Type::U32; // @bool
    } break;

    case Expression_Kind::And: {
        Type lhs_type = check_types_of_expression(expression->binary_op.lhs);
        Type rhs_type = check_types_of_expression(expression->binary_op.rhs);
        // @bool
        check_types(expression->binary_op.lhs->offset, Type::U32, lhs_type);
        check_types(expression->binary_op.rhs->offset, Type::U32, rhs_type);
        expression->type = Type::U32;
    } break;
    }

    return expression->type;
}

Type Type_Checker::check_types_of_statement(Statement *statement)
{
    switch (statement->kind) {
    case Statement_Kind::Local_Var_Def:
        statement->type = check_types_of_local_var_def(&statement->local_var_def);
        break;
    case Statement_Kind::While:
        statement->type = check_types_of_while(&statement->hwile);
        break;
    case Statement_Kind::If:
        statement->type = check_types_of_if(&statement->iph);
        break;
    case Statement_Kind::Assignment:
        statement->type = check_types_of_assignment(&statement->assignment);
        break;
    case Statement_Kind::Subtract_Assignment:
        statement->type = check_types_of_subtract_assignment(&statement->subtract_assignment);
        break;
    case Statement_Kind::Expression:
        statement->type = check_types_of_expression(statement->expression);
        break;
    }

    return statement->type;
}

Type Type_Checker::check_types_of_block(Block *block)
{
    push_scope();
    Type last_type = Type::Unchecked;
    while (block) {
        last_type = check_types_of_statement(&block->statement);
        block = block->next;
    }
    pop_scope();
    return last_type;
}

void Type_Checker::check_types_of_func_def(Func_Def *func_def)
{
    push_scope(func_def->args_list);

    auto body_type = check_types_of_block(func_def->body);

    if (body_type != func_def->return_type) {
        // TODO: unexpected return function type should have offset of the last (or relevant) statement in the body
        reporter.fail(func_def->offset, "Expected return type `", func_def->return_type, "`, but the function returns `", body_type, "`");
    }

    pop_scope();
}

void Type_Checker::check_types_of_module(Module *module)
{
    auto top_level_def = module->top_level_defs;
    while (top_level_def) {
        check_types_of_func_def(&top_level_def->func_def);
        top_level_def = top_level_def->next;
    }
}
