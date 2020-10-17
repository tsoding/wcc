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
            return true;
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
            return true;
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
            return true;
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

void Type_Checker::check_types_of_local_var_def(Local_Var_Def *local_var_def, Type expected_type)
{
    local_var_def->value = try_implicitly_cast_expression_to(
        check_types_of_expression(local_var_def->value),
        local_var_def->def.type);

    const auto TYPE_OF_LOCAL_VAR_DEF = Type::U0;

    if (expected_type != TYPE_OF_LOCAL_VAR_DEF) {
        // TODO: use offset of the local_var_def itself instead of its value;
        reporter.fail(local_var_def->value->offset, "Expected statement of type `", expected_type, "`, but local variable definition has the type `", TYPE_OF_LOCAL_VAR_DEF, "`");
    }

    push_var_def(local_var_def->def);
}

void Type_Checker::check_types_of_while(While *hwile, Type expected_type)
{
    // TODO: support for booleans (grep for @bool)
    hwile->condition = try_implicitly_cast_expression_to(
        check_types_of_expression(hwile->condition),
        Type::U32);
    // TODO: use offset of the hwile body itself instead of its condition
    check_types_of_block(hwile->condition->offset, hwile->body, Type::U0);

    const auto TYPE_OF_WHILE = Type::U0;
    if (expected_type != TYPE_OF_WHILE) {
        // TODO: use offset of the hwile itself instead of its condition;
        reporter.fail(hwile->condition->offset, "Expected statement of type `", expected_type, "`, but while-loop construction has the type `", TYPE_OF_WHILE, "`");
    }
}

void Type_Checker::check_types_of_if(If *iph, Type expected_type)
{
    // @bool
    iph->condition = try_implicitly_cast_expression_to(
        check_types_of_expression(iph->condition),
        Type::U32);
    // TODO: use offsets of the `then` and `else` blocks themselves instead of the if condition
    check_types_of_block(iph->condition->offset, iph->then, expected_type);
    check_types_of_block(iph->condition->offset, iph->elze, expected_type);
}

void Type_Checker::check_types_of_assignment(Assignment *assignment, Type expected_type)
{
    // TODO: check_types_of_assignment should report errors on the assignment's offset, not its value one
    // TODO: assigment could be an expression that has the type of the variable it assigns
    size_t offset = assignment->value->offset;
    assignment->value = try_implicitly_cast_expression_to(
        check_types_of_expression(assignment->value),
        type_of_name(offset, assignment->var_name));

    const auto TYPE_OF_ASSIGNMENT = Type::U0;
    if (expected_type != TYPE_OF_ASSIGNMENT) {
        // TODO: use offset of the assignment itself instead of its value
        reporter.fail(assignment->value->offset, "Expected statement of type `", expected_type, "`, but assignment has the type `", TYPE_OF_ASSIGNMENT, "`");
    }
}

void Type_Checker::check_types_of_subtract_assignment(Subtract_Assignment *subtract_assignment, Type expected_type)
{
    // @subass-sugar
    size_t offset = subtract_assignment->value->offset;
    subtract_assignment->value = try_implicitly_cast_expression_to(
        check_types_of_expression(subtract_assignment->value),
        type_of_name(offset, subtract_assignment->var_name));

    const auto TYPE_OF_SUBTRACT_ASSIGNMENT = Type::U0;
    if (expected_type != TYPE_OF_SUBTRACT_ASSIGNMENT) {
        // TODO: use offset of the assignment itself instead of its value
        reporter.fail(subtract_assignment->value->offset, "Expected statement of type `", expected_type, "`, but subtract assignment has the type `", TYPE_OF_SUBTRACT_ASSIGNMENT, "`");
    }
}

Expression *Type_Checker::check_types_of_expression(Expression *expression)
{
    assert(expression->type == Type::Unchecked);

    switch (expression->kind) {
    case Expression_Kind::Type_Cast: {
        expression->type_cast.expression =
            cast_expression_to(
                check_types_of_expression(expression->type_cast.expression),
                expression->type_cast.type);
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
        expression->binary_op.lhs = check_types_of_expression(expression->binary_op.lhs);
        expression->binary_op.rhs = check_types_of_expression(expression->binary_op.rhs);
        auto lhs_type = expression->binary_op.lhs->type;
        auto rhs_type = expression->binary_op.rhs->type;

        if (is_expression_castable_to(expression->binary_op.lhs, rhs_type)) {
            expression->binary_op.lhs = cast_expression_to(expression->binary_op.lhs, rhs_type);
            expression->type = rhs_type;
        } else if (is_expression_castable_to(expression->binary_op.rhs, lhs_type)) {
            expression->binary_op.rhs = cast_expression_to(expression->binary_op.rhs, lhs_type);
            expression->type = lhs_type;
        } else {
            reporter.fail(expression->binary_op.rhs->offset, "Types `", lhs_type, "` and `", rhs_type, "` are not comparable with each other");
        }
    } break;

    case Expression_Kind::Equals:
    case Expression_Kind::Less_Equals:
    case Expression_Kind::Greater: {
        expression->binary_op.lhs = check_types_of_expression(expression->binary_op.lhs);
        expression->binary_op.rhs = check_types_of_expression(expression->binary_op.rhs);
        auto lhs_type = expression->binary_op.lhs->type;
        auto rhs_type = expression->binary_op.rhs->type;

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
        expression->binary_op.lhs = try_implicitly_cast_expression_to(
            check_types_of_expression(expression->binary_op.lhs),
            Type::U32);
        expression->binary_op.rhs = try_implicitly_cast_expression_to(
            check_types_of_expression(expression->binary_op.rhs),
            Type::U32);
        expression->type = Type::U32; // @bool
    } break;
    }

    return expression;
}

void Type_Checker::check_types_of_statement(Statement *statement, Type expected_type)
{
    switch (statement->kind) {
    case Statement_Kind::Local_Var_Def:
        check_types_of_local_var_def(&statement->local_var_def, expected_type);
        break;
    case Statement_Kind::While:
        check_types_of_while(&statement->hwile, expected_type);
        break;
    case Statement_Kind::If:
        check_types_of_if(&statement->iph, expected_type);
        break;
    case Statement_Kind::Assignment:
        check_types_of_assignment(&statement->assignment, expected_type);
        break;
    case Statement_Kind::Subtract_Assignment:
        check_types_of_subtract_assignment(&statement->subtract_assignment, expected_type);
        break;
    case Statement_Kind::Expression:
        statement->expression = try_implicitly_cast_expression_to(
            check_types_of_expression(statement->expression),
            expected_type);
        break;
    }

    statement->type = expected_type;
}

void Type_Checker::check_types_of_block(size_t offset, Block *block, Type expected_type)
{
    push_scope();

    if (!block) {
        const Type TYPE_OF_EMPTY_BLOCK = Type::U0;
        if (expected_type != TYPE_OF_EMPTY_BLOCK) {
            reporter.fail(offset, "Expected type `", expected_type, "` but empty block has type `", TYPE_OF_EMPTY_BLOCK, "`");
        }
    }

    while (block && block->next != nullptr) {
        check_types_of_statement(&block->statement, Type::U0);
        block = block->next;
    }

    if (block) {
        check_types_of_statement(&block->statement, expected_type);
    }

    pop_scope();
}

void Type_Checker::check_types_of_func_def(Func_Def *func_def)
{
    push_scope(func_def->args_list);
    // TODO: use offset of the body instead of func_def
    check_types_of_block(func_def->offset, func_def->body, func_def->return_type);
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
