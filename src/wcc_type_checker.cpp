#include "./wcc_type_checker.hpp"

Maybe<Type> Type_Checker::type_of_name(String_View name) const
{
    auto scope0 = scope;

    while (scope0) {
        Args_List *args_list = scope0->args_list;
        while (args_list) {
            if (name == args_list->var_def.name) {
                return {true, args_list->var_def.type};
            }
            args_list = args_list->next;
        }
        scope0 = scope0->next;
    }

    return {};
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

Type Type_Checker::check_types_of_local_var_def(Local_Var_Def *local_var_def)
{
    return Type::Unchecked;
}

Type Type_Checker::check_types_of_while(While *hwile)
{
    return Type::Unchecked;
}

Type Type_Checker::check_types_of_assignment(Assignment *assignment)
{
    return Type::Unchecked;
}

Type Type_Checker::check_types_of_subtract_assignment(Subtract_Assignment *subtract_assignment)
{
    return Type::Unchecked;
}

Type Type_Checker::check_types_of_expression(Expression *expression)
{
    return Type::Unchecked;
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
