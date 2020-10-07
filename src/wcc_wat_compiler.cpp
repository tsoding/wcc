#include "wcc_wat_compiler.hpp"

void print1(FILE *stream, S_Expr *expr)
{
    if (expr == nullptr) {
        print(stream, "()");
    } else {
        switch (expr->type) {
        case S_Expr_Type::Atom: {
            print(stream, expr->atom.name);
        } break;

        case S_Expr_Type::Cons: {
            print(stream, "(");
            bool first = false;
            while (expr && expr->type == S_Expr_Type::Cons) {
                if (!first) {
                    first = true;
                } else {
                    print(stream, " ");
                }
                print(stream, expr->cons.head);
                expr = expr->cons.tail;
            }

            if (expr) {
                print(stream, " . ", expr);
            }

            print(stream, ")");
        } break;
        }
    }
}

S_Expr *Wat_Compiler::wat_ident(String_View s)
{
    return atom(concat("$"_sv, s));
}

S_Expr *Wat_Compiler::wat_name_of_type(Type type)
{
    switch (type) {
    case Type::U32:
        return atom("i32"_sv);
        break;

    case Type::U64:
        return atom("i64"_sv);
        break;
    }
    return {};
}

S_Expr *Wat_Compiler::compile_var_def(Var_Def var_def)
{
    return list(atom("param"_sv),
                wat_ident(var_def.name),
                wat_name_of_type(var_def.type));
}

S_Expr *Wat_Compiler::compile_args_list(Args_List *args_list)
{
    S_Expr *result = nullptr;
    S_Expr *last = nullptr;

    while (args_list) {
        S_Expr *node = cons(compile_var_def(args_list->var_def), nullptr);

        if (result == nullptr) {
            result = node;
        } else {
            assert(last);
            assert(last->type == S_Expr_Type::Cons);
            last->cons.tail = node;
        }

        last = node;

        args_list = args_list->next;
    }

    return result;
}

S_Expr *Wat_Compiler::compile_return_type(Type type)
{
    return list(atom("result"_sv), wat_name_of_type(type));
}

S_Expr *Wat_Compiler::compile_number_literal(Number_Literal number_literal)
{
    return list(atom("i32.const"_sv), atom(number_literal.unwrap));
}

S_Expr *Wat_Compiler::compile_variable(Variable variable)
{
    return list(atom("get_local"_sv), wat_ident(variable.name));
}

S_Expr *Wat_Compiler::compile_plus(Plus plus)
{
    return list(atom("i32.add"_sv), compile_expression(plus.lhs), compile_expression(plus.rhs));
}

S_Expr *Wat_Compiler::compile_greater(Greater greater)
{
    return list(atom("i32.gt_u"_sv), compile_expression(greater.lhs), compile_expression(greater.rhs));
}

S_Expr *Wat_Compiler::compile_expression(Expression *expression)
{
    switch (expression->type) {
    case Expression_Type::Number_Literal:
        return compile_number_literal(expression->number_literal);
    case Expression_Type::Variable:
        return compile_variable(expression->variable);
    case Expression_Type::Plus:
        return compile_plus(expression->plus);
    case Expression_Type::Greater:
        return compile_greater(expression->greater);
    }

    assert(0 && "Unknown type of expression");
    return nullptr;
}

S_Expr *Wat_Compiler::compile_statement(Statement statement)
{
    switch (statement.type) {
    case Statement_Type::Local_Var_Def:
        fail(statement.offset, "TODO: Local variable definitions are only allowed at the beginning of the function");
        break;
    case Statement_Type::While:
        // (block
        //  (loop
        //   (br_if 1 (i32.eqz <condition>))
        //   <body>
        //   (br 0)))
        return list(
            atom("block"_sv),
            append(
                list(atom("loop"_sv),
                     list(atom("br_if"_sv), atom(1),
                          list(atom("i32.eqz"_sv), compile_expression(statement.hwile.condition)))),
                compile_block(statement.hwile.body),
                list(list(atom("br"_sv), atom(0)))));
    case Statement_Type::Assignment:
        return list(atom("set_local"_sv),
                    wat_ident(statement.assignment.var_name),
                    compile_expression(statement.assignment.value));
    case Statement_Type::Subtract_Assignment: {
        // TODO: can we get rid of Subtract_Assignment and simply desugar it during the parsing?
        auto var_ident = wat_ident(statement.subtract_assignment.var_name);
        return list(atom("set_local"_sv),
                    var_ident,
                    list(atom("i32.sub"_sv),
                         list(atom("get_local"_sv), var_ident),
                         compile_expression(statement.subtract_assignment.value)));
    }
    case Statement_Type::Expression:
        return compile_expression(statement.expression);
    }

    return atom("<statement>"_sv);
}

S_Expr *Wat_Compiler::compile_block(Block *block)
{
    S_Expr *result = nullptr;

    while (block) {
        result = append(result, list(compile_statement(block->statement)));
        block = block->next;

    }
    return result;
}

S_Expr *Wat_Compiler::compile_func_body(Block *block)
{
    S_Expr *local_var_def_section = nullptr;
    S_Expr *local_var_def_init = nullptr;

    while (block && block->statement.type == Statement_Type::Local_Var_Def) {
        auto var_ident = wat_ident(block->statement.local_var_def.def.name);

        local_var_def_section = append(
            local_var_def_section,
            list(list(atom("local"_sv),
                      var_ident,
                      wat_name_of_type(block->statement.local_var_def.def.type))));

        local_var_def_init = append(
            local_var_def_init,
            list(list(atom("set_local"_sv),
                      var_ident,
                      compile_expression(block->statement.local_var_def.value))));

        block = block->next;
    }

    return append(local_var_def_section, local_var_def_init, compile_block(block));
}

S_Expr *Wat_Compiler::compile_func_def(Func_Def func_def)
{
    return append(
        list(atom("func"_sv),
             atom(concat("$"_sv, func_def.name)),
             list(atom("export"_sv), atom(concat("\""_sv, func_def.name, "\""_sv)))),
        compile_args_list(func_def.args_list),
        list(compile_return_type(func_def.return_type)),
        compile_func_body(func_def.body));
}

S_Expr *Wat_Compiler::compile_module(Module module)
{
    S_Expr *result = nullptr;

    Top_Level_Def *top_level_def = module.top_level_defs;
    while (top_level_def) {
        result = append(result, list(compile_func_def(top_level_def->func_def)));
        top_level_def = top_level_def->next;
    }

    return cons(atom("module"_sv), result);
}
