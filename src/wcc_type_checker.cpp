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

// TODO: Consider removing implicit conversion from any type to u0
//
// Introduce a compile intrinsic `drop()`, so if you wanna have a
// expression in the position of u0 you have to do
// `drop(<expression>)`. Similarly to how you would do that in raw
// wasm.

static const Primitive_Type implicit_conversion_table[][2] = {
    {Primitive_Type::U0, Primitive_Type::U0},

    {Primitive_Type::U8, Primitive_Type::U0},
    {Primitive_Type::U8, Primitive_Type::U8},
    {Primitive_Type::U8, Primitive_Type::U32},
    {Primitive_Type::U8, Primitive_Type::U64},

    {Primitive_Type::U32, Primitive_Type::U0},
    {Primitive_Type::U32, Primitive_Type::U32},
    {Primitive_Type::U32, Primitive_Type::U64},

    {Primitive_Type::U64, Primitive_Type::U0},
    {Primitive_Type::U64, Primitive_Type::U64},

    {Primitive_Type::Bool, Primitive_Type::Bool},
};
const size_t implicit_conversion_table_count =
    sizeof(implicit_conversion_table) / sizeof(implicit_conversion_table[0]);

bool is_expression_implicitly_convertible_to(Expression *expression, Type target_type)
{
    assert(expression->type.kind != Type_Kind::Unchecked && "Unchecked type in a checked context");
    assert(target_type.kind != Type_Kind::Unchecked && "Unchecked type in a checked context");

    if (expression->type == target_type) {
        return true;
    }

    if (expression->type.kind == Type_Kind::Primitive_Type) {
        if (target_type.kind == Type_Kind::Primitive_Type) {
            for (size_t i = 0; i < implicit_conversion_table_count; ++i) {
                const size_t SOURCE = 0;
                const size_t TARGET = 1;
                if (implicit_conversion_table[i][SOURCE] == expression->type.primitive_type &&
                    implicit_conversion_table[i][TARGET] == target_type.primitive_type) {
                    return true;
                }
            }
        }
    }

    return false;
}

Expression *Type_Checker::try_implicitly_convert_expression_to(Expression *expression, Type cast_type)
{
    if (is_expression_implicitly_convertible_to(expression, cast_type)) {
        return cast_expression_to(expression, cast_type);
    } else {
        reporter.fail(expression->offset, "Expression of type `", expression->type, "` is not convertable to `", cast_type, "`");
    }

    return expression;
}

void Type_Checker::check_types_of_local_var_def(Local_Var_Def *local_var_def, Type expected_type)
{
    local_var_def->value = try_implicitly_convert_expression_to(
        check_types_of_expression(local_var_def->value),
        local_var_def->def.type);

    constexpr auto TYPE_OF_LOCAL_VAR_DEF = u0_type();

    if (expected_type != TYPE_OF_LOCAL_VAR_DEF) {
        // TODO: use offset of the local_var_def itself instead of its value;
        reporter.fail(local_var_def->value->offset, "Expected statement of type `", expected_type, "`, but local variable definition has the type `", TYPE_OF_LOCAL_VAR_DEF, "`");
    }

    push_var_def(local_var_def->def);
}

void Type_Checker::check_types_of_while(While *hwile, Type expected_type)
{
    hwile->condition = try_implicitly_convert_expression_to(
        check_types_of_expression(hwile->condition),
        bool_type());
    // TODO: use offset of the hwile body itself instead of its condition
    check_types_of_block(hwile->condition->offset, hwile->body, u0_type());

    const auto TYPE_OF_WHILE = u0_type();
    if (expected_type != TYPE_OF_WHILE) {
        // TODO: use offset of the hwile itself instead of its condition;
        reporter.fail(hwile->condition->offset, "Expected statement of type `", expected_type, "`, but while-loop construction has the type `", TYPE_OF_WHILE, "`");
    }
}

void Type_Checker::check_types_of_if(If *iph, Type expected_type)
{
    iph->condition = try_implicitly_convert_expression_to(
        check_types_of_expression(iph->condition),
        bool_type());
    // TODO: use offsets of the `then` and `else` blocks themselves instead of the if condition
    check_types_of_block(iph->condition->offset, iph->then, expected_type);
    check_types_of_block(iph->condition->offset, iph->elze, expected_type);
}

void Type_Checker::check_types_of_assignment(Assignment *assignment, Type expected_type)
{
    // TODO: check_types_of_assignment should report errors on the assignment's offset, not its value one
    // TODO: assigment could be an expression that has the type of the variable it assigns
    size_t offset = assignment->value->offset;
    assignment->value = try_implicitly_convert_expression_to(
        check_types_of_expression(assignment->value),
        type_of_name(offset, assignment->var_name));

    const auto TYPE_OF_ASSIGNMENT = u0_type();
    if (expected_type != TYPE_OF_ASSIGNMENT) {
        // TODO: use offset of the assignment itself instead of its value
        reporter.fail(assignment->value->offset, "Expected statement of type `", expected_type, "`, but assignment has the type `", TYPE_OF_ASSIGNMENT, "`");
    }
}

void Type_Checker::check_types_of_func_call(Func_Call *)
{
    assert(0 && "TODO: Type_Checker::check_types_of_func_call is not implemented");
}

Expression *Type_Checker::check_types_of_expression(Expression *expression)
{
    assert(expression->type.kind == Type_Kind::Unchecked);

    switch (expression->kind) {
    case Expression_Kind::Func_Call: {
        check_types_of_func_call(&expression->func_call);
    } break;

    case Expression_Kind::Type_Cast: {
        expression->type_cast.expression =
            cast_expression_to(
                check_types_of_expression(expression->type_cast.expression),
                expression->type_cast.type);
        expression->type = expression->type_cast.type;
    } break;

    case Expression_Kind::Number_Literal: {
        if (expression->number_literal.unwrap <= 0xFF) {
            expression->type = u8_type();
        } else if (expression->number_literal.unwrap <= 0xFF'FF'FF'FF) {
            expression->type = u32_type();
        } else {
            expression->type = u64_type();
        }
    } break;

    case Expression_Kind::Bool_Literal: {
        expression->type = bool_type();
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

        if (is_expression_implicitly_convertible_to(expression->binary_op.lhs, rhs_type)) {
            expression->binary_op.lhs = cast_expression_to(expression->binary_op.lhs, rhs_type);
            expression->type = rhs_type;
        } else if (is_expression_implicitly_convertible_to(expression->binary_op.rhs, lhs_type)) {
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

        if (is_expression_implicitly_convertible_to(expression->binary_op.lhs, rhs_type)) {
            expression->binary_op.lhs = cast_expression_to(expression->binary_op.lhs, rhs_type);
        } else if (is_expression_implicitly_convertible_to(expression->binary_op.lhs, lhs_type)) {
            expression->binary_op.rhs = try_implicitly_convert_expression_to(expression->binary_op.rhs, lhs_type);
        } else {
            reporter.fail(expression->binary_op.rhs->offset, "Types `", lhs_type, "` and `", rhs_type, "` are not comparable with each other");
        }
        expression->type = bool_type();
    } break;

    case Expression_Kind::And: {
        expression->binary_op.lhs = try_implicitly_convert_expression_to(
            check_types_of_expression(expression->binary_op.lhs),
            bool_type());
        expression->binary_op.rhs = try_implicitly_convert_expression_to(
            check_types_of_expression(expression->binary_op.rhs),
            bool_type());
        expression->type = bool_type();
    } break;
    }

    return expression;
}

void Type_Checker::check_types_of_return(Return *reeturn, Type expected_type)
{
    assert(current_func_def && "Type checking return statement outside of a Func_Def context");
    reeturn->value = try_implicitly_convert_expression_to(
        check_types_of_expression(reeturn->value),
        current_func_def->return_type);

    const auto TYPE_OF_RETURN = u0_type();
    if (expected_type != TYPE_OF_RETURN) {
        // TODO: use offset of the assignment itself instead of its value
        reporter.fail(reeturn->value->offset, "Expected statement of type `", expected_type, "`, but return has the type `", TYPE_OF_RETURN, "`");
    }
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
    case Statement_Kind::Expression:
        statement->expression = try_implicitly_convert_expression_to(
            check_types_of_expression(statement->expression),
            expected_type);
        break;
    case Statement_Kind::Return:
        check_types_of_return(&statement->reeturn, expected_type);
        break;
    }

    statement->type = expected_type;
}

void Type_Checker::check_types_of_block(size_t offset, Block *block, Type expected_type)
{
    push_scope();

    if (!block) {
        const Type TYPE_OF_EMPTY_BLOCK = u0_type();
        if (expected_type != TYPE_OF_EMPTY_BLOCK) {
            reporter.fail(offset, "Expected type `", expected_type, "` but empty block has type `", TYPE_OF_EMPTY_BLOCK, "`");
        }
    }

    while (block && block->next != nullptr) {
        check_types_of_statement(&block->statement, u0_type());
        block = block->next;
    }

    if (block) {
        check_types_of_statement(&block->statement, expected_type);
    }

    pop_scope();
}

void Type_Checker::check_types_of_func_def(Func_Def *func_def)
{
    assert(current_func_def == nullptr &&
           "Checking types of Func_Def in the contenxt of other Func_Def. This kinda stuff is not supported yet.");
    current_func_def = func_def;
    push_scope(func_def->args_list);
    // TODO: use offset of the body instead of func_def
    check_types_of_block(func_def->offset, func_def->body, func_def->return_type);
    current_func_def = nullptr;
    pop_scope();
}

void Type_Checker::check_types_of_include(Include *)
{
    assert(0 && "Type_Checker::check_types_of_include() is not implemented");
}

void Type_Checker::check_types_of_top_def(Top_Def *top_def)
{
    switch (top_def->kind) {
    case Top_Def_Kind::Func_Def:
        check_types_of_func_def(&top_def->func_def);
        break;

    case Top_Def_Kind::Include:
        check_types_of_include(&top_def->include);
        break;
    }
}

void Type_Checker::check_types_of_module(Module *module)
{
    auto top_defs = module->top_defs.begin;
    while (top_defs) {
        check_types_of_top_def(&top_defs->unwrap);
        top_defs = top_defs->next;
    }
}
