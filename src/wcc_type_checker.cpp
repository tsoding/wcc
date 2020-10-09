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

void Type_Checker::check_types(size_t offset, Type expected_type, Type actual_type)
{
    if (expected_type != actual_type) {
        reporter.fail(offset, "Expected type `", expected_type, "` but got `", actual_type, "`");
    }
}

Type Type_Checker::check_types_of_local_var_def(Local_Var_Def *local_var_def)
{
    check_types(local_var_def->value->offset, local_var_def->def.type, check_types_of_expression(local_var_def->value));
    push_var_def(local_var_def->def);
    return local_var_def->def.type;
}

Type Type_Checker::check_types_of_while(While *hwile)
{
    // TODO: support for booleans @bool
    check_types(hwile->condition->offset, Type::U32, check_types_of_expression(hwile->condition));
    check_types_of_block(hwile->body);
    return Type::U0;
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
    check_types(
        offset,
        type_of_name(offset, subtract_assignment->var_name),
        check_types_of_expression(subtract_assignment->value));
    return Type::U0;
}

Type Type_Checker::check_types_of_expression(Expression *expression)
{
    assert(expression->type == Type::Unchecked);

    switch (expression->kind) {
    case Expression_Kind::Number_Literal: {
        expression->type = Type::U32;
    } break;

    case Expression_Kind::Variable: {
        expression->type = type_of_name(expression->offset, expression->variable.name);
    } break;

    case Expression_Kind::Plus: {
        Type lhs_type = check_types_of_expression(expression->plus.lhs);
        Type rhs_type = check_types_of_expression(expression->plus.rhs);
        check_types(expression->plus.rhs->offset, lhs_type, rhs_type);
        expression->type = lhs_type;
    } break;

    case Expression_Kind::Greater: {
        Type lhs_type = check_types_of_expression(expression->plus.lhs);
        Type rhs_type = check_types_of_expression(expression->plus.rhs);
        check_types(expression->plus.rhs->offset, lhs_type, rhs_type);
        expression->type = Type::U32; // @bool
    } break;
    }

    return expression->type;
}

Type Type_Checker::check_types_of_statement(Statement *statement)
{
    switch (statement->kind) {
    case Statement_Kind::Local_Var_Def:
        return check_types_of_local_var_def(&statement->local_var_def);
    case Statement_Kind::While:
        return check_types_of_while(&statement->hwile);
    case Statement_Kind::Assignment:
        return check_types_of_assignment(&statement->assignment);
    case Statement_Kind::Subtract_Assignment:
        return check_types_of_subtract_assignment(&statement->subtract_assignment);
    case Statement_Kind::Expression:
        return check_types_of_expression(statement->expression);
    }

    assert(0 && "This can only happen if the memory is corrupted");
    return Type::Unchecked;
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