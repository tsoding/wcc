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
    switch (type.kind) {
    case Type_Kind::Unchecked:
        assert(0 && "Something went horribly wrong. Such value should never reach here. Trying to compile a type that does not have a WebAssembly representation.");
        break;

    case Type_Kind::Primitive_Type:
        switch (type.primitive_type) {
        case Primitive_Type::U0:
            assert(0 && "Something went horribly wrong. Such value should never reach here. Trying to compile a type that does not have a WebAssembly representation.");
            break;

        case Primitive_Type::U8:
            return atom("i32"_sv);

        case Primitive_Type::U32:
            return atom("i32"_sv);

        case Primitive_Type::U64:
            return atom("i64"_sv);

        case Primitive_Type::Bool:
            return atom("i32"_sv);
        }
        break;

    case Type_Kind::Pointer:
        return atom("i32"_sv);
    }
    return nullptr;
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

S_Expr *Wat_Compiler::compile_bool_literal(Bool_Literal bool_literal)
{
    if (bool_literal.unwrap) {
        return list(atom("i32.const"_sv), atom(1));
    } else {
        return list(atom("i32.const"_sv), atom(0));
    }
}

S_Expr *Wat_Compiler::compile_variable(Variable variable)
{
    return list(atom("get_local"_sv), wat_ident(variable.name));
}

S_Expr *Wat_Compiler::compile_rem(Binary_Op rem)
{
    assert(rem.lhs->type == rem.rhs->type && "Type Checking step didn't work correctly. Left and right operands of Rem operation should have the same type");

    switch (rem.lhs->type.kind) {
    case Type_Kind::Unchecked:
        assert(0 && "Unchecked type in a checked context");
        break;

    case Type_Kind::Primitive_Type:
        switch (rem.lhs->type.primitive_type) {
        case Primitive_Type::U0:
            assert(0 && "Type checking did not work correctly. There is no Rem operation defined for u0.");
            break;
        case Primitive_Type::Bool:
            assert(0 && "Type checking did not work correctly. There is no Rem operation defined for bool.");
            break;
        case Primitive_Type::U8:
        case Primitive_Type::U32:
            return list(atom("i32.rem_u"_sv), compile_expression(rem.lhs), compile_expression(rem.rhs));
        case Primitive_Type::U64:
            return list(atom("i64.rem_u"_sv), compile_expression(rem.lhs), compile_expression(rem.rhs));
        }
        break;

    case Type_Kind::Pointer:
        assert(0 && "Type checking did not work correctly. There is no Rem operation defined for pointers.");
        break;
    }

    assert(0 && "Memory corruption?");
    return nullptr;
}

S_Expr *Wat_Compiler::compile_minus(Binary_Op minus)
{
    assert(minus.lhs->type == minus.rhs->type && "Type Checking step didn't work correctly. Left and right operands of Minus operation should have the same type");

    switch (minus.lhs->type.kind) {
    case Type_Kind::Unchecked:
        assert(0 && "Unchecked type in a checked context");
        break;

    case Type_Kind::Primitive_Type:
        switch (minus.lhs->type.primitive_type) {
        case Primitive_Type::U0:
            assert(0 && "Type Checking step didn't work correctly. There is no minus operation defined for u0.");
            break;
        case Primitive_Type::Bool:
            assert(0 && "Type Checking step didn't work correctly. There is no minus operation defined for bool.");
            break;
        case Primitive_Type::U8:
        case Primitive_Type::U32:
            return list(atom("i32.sub"_sv), compile_expression(minus.lhs), compile_expression(minus.rhs));
        case Primitive_Type::U64:
            return list(atom("i64.sub"_sv), compile_expression(minus.lhs), compile_expression(minus.rhs));
        }
        break;

    case Type_Kind::Pointer:
        assert(0 && "Type Checking step didn't work correctly. There is no minus operation defined for pointers.");
        break;
    }

    assert(0 && "Memory corruption?");
    return nullptr;
}

S_Expr *Wat_Compiler::compile_multiply(Binary_Op multiply)
{
    assert(multiply.lhs->type == multiply.rhs->type && "Type Checking step didn't work correctly. Left and right operands of Minus operation should have the same type");

    switch (multiply.lhs->type.kind) {
    case Type_Kind::Unchecked:
        assert(0 && "Unchecked type in a checked context");
        break;
    case Type_Kind::Primitive_Type:
        switch (multiply.lhs->type.primitive_type) {
        case Primitive_Type::U0:
            assert(0 && "Type Checking step didn't work correctly. There is no multiply operation defined for u0.");
            break;
        case Primitive_Type::Bool:
            assert(0 && "Type Checking step didn't work correctly. There is no multiply operation defined for bool.");
            break;
        case Primitive_Type::U8:
        case Primitive_Type::U32:
            return list(atom("i32.mul"_sv), compile_expression(multiply.lhs), compile_expression(multiply.rhs));
        case Primitive_Type::U64:
            return list(atom("i64.mul"_sv), compile_expression(multiply.lhs), compile_expression(multiply.rhs));
        }
        break;

    case Type_Kind::Pointer:
        assert(0 && "Type Checking step didn't work correctly. There is no multiply operation defined for pointer.");
        break;
    }

    assert(0 && "Memory corruption?");
    return nullptr;
}

S_Expr *Wat_Compiler::compile_plus(Binary_Op plus)
{
    assert(plus.lhs->type == plus.rhs->type && "Type Checking step didn't work correctly. Left and right operands of Minus operation should have the same type");

    switch (plus.lhs->type.kind) {
    case Type_Kind::Unchecked:
        assert(0 && "Unchecked type in a checked context");

    case Type_Kind::Primitive_Type:
        switch (plus.lhs->type.primitive_type) {
        case Primitive_Type::U0:
            assert(0 && "Type Checking step didn't work correctly. There is no plus operation defined for u0");
            break;
        case Primitive_Type::Bool:
            assert(0 && "Type Checking step didn't work correctly. There is no plus operation defined for bool");
            break;
        case Primitive_Type::U8:
        case Primitive_Type::U32:
            return list(atom("i32.add"_sv), compile_expression(plus.lhs), compile_expression(plus.rhs));
        case Primitive_Type::U64:
            return list(atom("i64.add"_sv), compile_expression(plus.lhs), compile_expression(plus.rhs));
        }
        break;

    case Type_Kind::Pointer:
        assert(0 && "Type Checking step didn't work correctly. There is no plus operation defined for pointer");
        break;
    }

    assert(0 && "Memory corruption?");
    return nullptr;
}

S_Expr *Wat_Compiler::compile_less_equals(Binary_Op less_equals)
{
    assert(less_equals.lhs->type == less_equals.rhs->type && "Type Checking step didn't work correctly. Left and right operands of Minus operation should have the same type");

    switch (less_equals.lhs->type.kind) {
    case Type_Kind::Unchecked:
        assert(0 && "Unchecked type in a checked context");
        break;
    case Type_Kind::Primitive_Type:
        switch (less_equals.lhs->type.primitive_type) {
        case Primitive_Type::U0:
            assert(0 && "Type Checking step didn't work correctly. There is no less_equals operation defined for u0");
            break;
        case Primitive_Type::Bool:
            assert(0 && "Type Checking step didn't work correctly. There is no less_equals operation defined for bool");
            break;
        case Primitive_Type::U8:
            return list(atom("i32.le_u"_sv), compile_expression(less_equals.lhs), compile_expression(less_equals.rhs));
        case Primitive_Type::U32:
            return list(atom("i32.le_u"_sv), compile_expression(less_equals.lhs), compile_expression(less_equals.rhs));
        case Primitive_Type::U64:
            return list(atom("i64.le_u"_sv), compile_expression(less_equals.lhs), compile_expression(less_equals.rhs));
        }
        break;
    case Type_Kind::Pointer:
        return list(atom("i32.le_u"_sv), compile_expression(less_equals.lhs), compile_expression(less_equals.rhs));
    }

    assert(0 && "Memory corruption?");
    return nullptr;
}

S_Expr *Wat_Compiler::compile_equals(Binary_Op equals)
{
    assert(equals.lhs->type == equals.rhs->type && "Type Checking step didn't work correctly. Left and right operands of Minus operation should have the same type");

    switch (equals.lhs->type.kind) {
    case Type_Kind::Unchecked:
        assert(0 && "Unchecked type in a checked context");
        break;
    case Type_Kind::Primitive_Type:
        switch (equals.lhs->type.primitive_type) {
        case Primitive_Type::U0:
            assert(0 && "Type Checking step didn't work correctly. There is no equals operation defined for u0");
        case Primitive_Type::Bool:
            assert(0 && "Type Checking step didn't work correctly. There is no equals operation defined for bool");
        case Primitive_Type::U8:
            return list(atom("i32.eq"_sv), compile_expression(equals.lhs), compile_expression(equals.rhs));
        case Primitive_Type::U32:
            return list(atom("i32.eq"_sv), compile_expression(equals.lhs), compile_expression(equals.rhs));
        case Primitive_Type::U64:
            return list(atom("i64.eq"_sv), compile_expression(equals.lhs), compile_expression(equals.rhs));
        }
        break;
    case Type_Kind::Pointer:
        return list(atom("i32.eq"_sv), compile_expression(equals.lhs), compile_expression(equals.rhs));
    }

    assert(0 && "Memory corruption?");
    return nullptr;
}

S_Expr *Wat_Compiler::compile_greater(Binary_Op greater)
{
    assert(greater.lhs->type == greater.rhs->type && "Type Checking step didn't work correctly. Left and right operands of Minus operation should have the same type");

    switch (greater.lhs->type.kind) {
    case Type_Kind::Unchecked:
        assert(0 && "Unchecked type in a checked context");
        break;

    case Type_Kind::Primitive_Type:
        switch (greater.lhs->type.primitive_type) {
        case Primitive_Type::U0:
            assert(0 && "Type Checking step didn't work correctly. There is no greater operation defined for u0");
        case Primitive_Type::Bool:
            assert(0 && "Type Checking step didn't work correctly. There is no greater operation defined for bool");
        case Primitive_Type::U8:
            return list(atom("i32.gt_u"_sv), compile_expression(greater.lhs), compile_expression(greater.rhs));
        case Primitive_Type::U32:
            return list(atom("i32.gt_u"_sv), compile_expression(greater.lhs), compile_expression(greater.rhs));
        case Primitive_Type::U64:
            return list(atom("i64.gt_u"_sv), compile_expression(greater.lhs), compile_expression(greater.rhs));
        }
        break;

    case Type_Kind::Pointer:
        return list(atom("i32.gt_u"_sv), compile_expression(greater.lhs), compile_expression(greater.rhs));
    }

    assert(0 && "Memory corruption?");
    return nullptr;
}

S_Expr *Wat_Compiler::compile_and(Binary_Op andd)
{
    assert(andd.lhs->type == andd.rhs->type && "Type Checking step didn't work correctly. Left and right operands of Minus operation should have the same type");

    switch (andd.lhs->type.kind) {
    case Type_Kind::Unchecked:
        assert(0 && "Unchecked type in a checked context");
        break;

    case Type_Kind::Primitive_Type:
        switch (andd.lhs->type.primitive_type) {
        case Primitive_Type::U0:
            assert(0 && "Type Checking step didn't work correctly. There is no and operation defined for u0");
            break;
        case Primitive_Type::U8:
            assert(0 && "Type Checking step didn't work correctly. There is no and operation defined for u8");
            break;
        case Primitive_Type::U32:
            assert(0 && "Type Checking step didn't work correctly. There is no and operation defined for u32");
            break;
        case Primitive_Type::U64:
            assert(0 && "Type Checking step didn't work correctly. There is no and operation defined for u64");
            break;
        case Primitive_Type::Bool:
            return list(atom("i32.and"_sv), compile_expression(andd.lhs), compile_expression(andd.rhs));
        }
        break;

    case Type_Kind::Pointer:
        assert(0 && "Type Checking step didn't work correctly. There is no and operation defined for pointer");
    }

    assert(0 && "Memory corruption?");
    return nullptr;
}

S_Expr *Wat_Compiler::compile_type_cast(Type_Cast type_cast)
{
    auto expression = compile_expression(type_cast.expression);

    switch (type_cast.expression->type.kind) {
    case Type_Kind::Primitive_Type:
        switch (type_cast.expression->type.primitive_type) {
        case Primitive_Type::U0:
            reporter.fail(type_cast.expression->offset, "Impossible to convert `", type_cast.expression->type, "` to `", type_cast.type, "`");

        case Primitive_Type::Bool:
            switch (type_cast.type.kind) {
            case Type_Kind::Primitive_Type:
                switch (type_cast.type.primitive_type) {
                case Primitive_Type::Bool:
                    return expression;
                case Primitive_Type::U0:
                case Primitive_Type::U8:
                case Primitive_Type::U32:
                case Primitive_Type::U64:
                    reporter.fail(type_cast.expression->offset, "Impossible to convert `", type_cast.expression->type, "` to `", type_cast.type, "`");
                    break;
                }
                break;
            case Type_Kind::Pointer:
                reporter.fail(type_cast.expression->offset, "Impossible to convert `", type_cast.expression->type, "` to `", type_cast.type, "`");
                break;

            case Type_Kind::Unchecked:
                assert(0 && "Unchecked type in a checked context");
                break;
            }
            break;

        case Primitive_Type::U8:
            switch (type_cast.type.kind) {
            case Type_Kind::Unchecked:
                assert(0 && "Unchecked type in a checked context");
                break;

            case Type_Kind::Primitive_Type:
                switch (type_cast.type.primitive_type) {
                case Primitive_Type::U0:
                    return list(atom("drop"_sv), expression);
                case Primitive_Type::U8:
                case Primitive_Type::U32:
                case Primitive_Type::Bool:
                    return expression;
                case Primitive_Type::U64:
                    return list(atom("i64.extend_u/i32"_sv), expression);
                }
                break;

            case Type_Kind::Pointer:
                return expression;
            }
            break;

        case Primitive_Type::U32:
            switch (type_cast.type.kind) {
            case Type_Kind::Primitive_Type:
                switch (type_cast.type.primitive_type) {
                case Primitive_Type::U0:
                    return list(atom("drop"_sv), expression);
                case Primitive_Type::U8:
                    reporter.fail(type_cast.expression->offset, "Impossible to convert `", type_cast.expression->type, "` to `", type_cast.type, "`");
                case Primitive_Type::Bool:
                case Primitive_Type::U32:
                    return expression;
                case Primitive_Type::U64:
                    return list(atom("i64.extend_u/i32"_sv), expression);
                }
                break;

            case Type_Kind::Pointer:
                return expression;

            case Type_Kind::Unchecked:
                assert(0 && "Unchecked type in a checked context");
                break;
            }
            break;

        case Primitive_Type::U64:
            switch (type_cast.type.kind) {
            case Type_Kind::Primitive_Type:
                switch (type_cast.type.primitive_type) {
                case Primitive_Type::U0:
                    return list(atom("drop"_sv), expression);
                case Primitive_Type::U8:
                case Primitive_Type::U32:
                case Primitive_Type::Bool:
                    reporter.fail(type_cast.expression->offset, "Impossible to convert `", type_cast.expression->type, "` to `", type_cast.type, "`");
                    break;
                case Primitive_Type::U64:
                    return expression;
                }
                break;

            case Type_Kind::Pointer:
                reporter.fail(type_cast.expression->offset, "Impossible to convert `", type_cast.expression->type, "` to `", type_cast.type, "`");
                break;

            case Type_Kind::Unchecked:
                assert(0 && "Unchecked type in a checked context");
                break;
            }
            break;
        }
        break;

    case Type_Kind::Pointer:
        reporter.fail(type_cast.expression->offset, "Impossible to convert `", type_cast.expression->type, "` to `", type_cast.type, "`");
        break;

    case Type_Kind::Unchecked:
        assert(0 && "Unchecked type in a checked context");
        break;
    }

    assert(0 && "Memory corruption?");
    return nullptr;
}

S_Expr *Wat_Compiler::compile_expression(Expression *expression)
{
    switch (expression->kind) {
    case Expression_Kind::Type_Cast:
        return compile_type_cast(expression->type_cast);
    case Expression_Kind::Number_Literal:
        return compile_number_literal(expression->number_literal);
    case Expression_Kind::Bool_Literal:
        return compile_bool_literal(expression->bool_literal);
    case Expression_Kind::Variable:
        return compile_variable(expression->variable);
    case Expression_Kind::Plus:
        return compile_plus(expression->binary_op);
    case Expression_Kind::Multiply:
        return compile_multiply(expression->binary_op);
    case Expression_Kind::Greater:
        return compile_greater(expression->binary_op);
    case Expression_Kind::Rem:
        return compile_rem(expression->binary_op);
    case Expression_Kind::Minus:
        return compile_minus(expression->binary_op);
    case Expression_Kind::And:
        return compile_and(expression->binary_op);
    case Expression_Kind::Less_Equals:
        return compile_less_equals(expression->binary_op);
    case Expression_Kind::Equals:
        return compile_equals(expression->binary_op);
    }

    assert(0 && "Memory corruption?");
    return nullptr;
}

S_Expr *Wat_Compiler::compile_statement(Statement statement)
{
    switch (statement.kind) {
    case Statement_Kind::Local_Var_Def:
        reporter.fail(statement.offset, "TODO: Local variable definitions are only allowed at the beginning of the function");
        break;
    case Statement_Kind::If:
        if (statement.iph.then == nullptr && statement.iph.elze == nullptr) {
            return list(atom("drop"_sv), compile_expression(statement.iph.condition));
        } else {
            //  (if (result <type>)
            //    (then <then-branch>)
            //    (else (else-branch)))
            if (statement.type.kind == Type_Kind::Primitive_Type &&
                statement.type.primitive_type == Primitive_Type::U0) {
                return list(
                    atom("if"_sv),
                    compile_expression(statement.iph.condition),
                    append(list(atom("then"_sv)), compile_block(statement.iph.then)),
                    append(list(atom("else"_sv)), compile_block(statement.iph.elze)));
            } else {
                return list(
                    atom("if"_sv), list(atom("result"_sv), wat_name_of_type(statement.type)),
                    compile_expression(statement.iph.condition),
                    append(list(atom("then"_sv)), compile_block(statement.iph.then)),
                    append(list(atom("else"_sv)), compile_block(statement.iph.elze)));
            }
        }
    case Statement_Kind::While:
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

    case Statement_Kind::Assignment:
        return list(atom("set_local"_sv),
                    wat_ident(statement.assignment.var_name),
                    compile_expression(statement.assignment.value));

    case Statement_Kind::Expression:
        return compile_expression(statement.expression);

    case Statement_Kind::Return:
        return list(atom("return"_sv), compile_expression(statement.reeturn.value));
    }

    assert(0 && "Memory corruption?");
    return nullptr;
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

    while (block && block->statement.kind == Statement_Kind::Local_Var_Def) {
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

S_Expr *Wat_Compiler::compile_include(Include)
{
    assert(0 && "TODO: Include compilation is not implemented");
    return nullptr;
}

S_Expr *Wat_Compiler::compile_module(Module module)
{
    S_Expr *result = nullptr;

    auto top_defs = module.top_defs;
    while (top_defs) {
        switch (top_defs->unwrap.kind) {
        case Top_Def_Kind::Func_Def:
            result = append(result, list(compile_func_def(top_defs->unwrap.func_def)));
            break;
        case Top_Def_Kind::Include:
            result = append(result, list(compile_include(top_defs->unwrap.include)));
            break;
        }
        top_defs = top_defs->next;
    }

    return cons(atom("module"_sv), result);
}
