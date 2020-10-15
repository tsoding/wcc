#ifndef WCC_WAT_COMPILER_HPP_
#define WCC_WAT_COMPILER_HPP_

struct S_Expr;

struct Cons
{
    S_Expr *head;
    S_Expr *tail;
};

struct Atom
{
    String_View name;
};

enum class S_Expr_Type
{
    Cons,
    Atom,
};

struct S_Expr
{
    S_Expr_Type type;
    union
    {
        Cons cons;
        Atom atom;
    };
};

void print1(FILE *stream, S_Expr *expr);

struct Wat_Compiler
{
    Linear_Memory *memory;
    Reporter reporter;

    S_Expr *atom(String_View name)
    {
        S_Expr *expr = memory->alloc<S_Expr>();
        expr->type = S_Expr_Type::Atom;
        expr->atom.name = name;
        return expr;
    }

    S_Expr *atom(uint32_t number)
    {
        String_Buffer buffer = {};
        buffer.capacity = 16;   // NOTE: 2^32 = 4294967296 so 16 chars should be more than enough
        buffer.data = memory->alloc<char>(16);
        sprint(&buffer, number);
        return atom(buffer.view());
    }

    S_Expr *cons(S_Expr *head, S_Expr *tail)
    {
        S_Expr *expr = memory->alloc<S_Expr>();
        expr->type = S_Expr_Type::Cons;
        expr->cons.head = head;
        expr->cons.tail = tail;
        return expr;
    }

    S_Expr *list()
    {
        return nullptr;
    }

    template <typename... Tail>
    S_Expr *list(S_Expr *head, Tail... tail)
    {
        return cons(head, list(tail...));
    }

    S_Expr *append()
    {
        return nullptr;
    }

    template <typename... Rest>
    S_Expr *append(S_Expr *list1, Rest... rest)
    {
        S_Expr *list2 = append(rest...);

        if (list1 == nullptr) {
            return list2;
        }

        S_Expr *last = list1;

        while (last != nullptr &&
               last->type == S_Expr_Type::Cons &&
               last->cons.tail != nullptr)
        {
            last = last->cons.tail;
        }

        assert(last);
        assert(last->type == S_Expr_Type::Cons);
        assert(last->cons.tail == nullptr);

        last->cons.tail = list2;

        return list1;
    }

    template <typename... Strings>
    String_View concat(Strings... s)
    {
        String_Buffer buffer = {};
        buffer.capacity = (s.count + ...) + 1;
        buffer.data = memory->alloc<char>(buffer.capacity);
        sprintln(&buffer, s...);
        return buffer.view();
    }

    S_Expr *wat_ident(String_View s);
    S_Expr *wat_name_of_type(Type type);

    S_Expr *compile_type_cast(Type_Cast type_cast);
    S_Expr *compile_var_def(Var_Def var_def);
    S_Expr *compile_args_list(Args_List *args_list);
    S_Expr *compile_return_type(Type type);
    S_Expr *compile_number_literal(Number_Literal number_literal);
    S_Expr *compile_variable(Variable variable);
    S_Expr *compile_rem(Binary_Op rem);
    S_Expr *compile_plus(Binary_Op plus);
    S_Expr *compile_minus(Binary_Op minus);
    S_Expr *compile_greater(Binary_Op greater);
    S_Expr *compile_expression(Expression *expression);
    S_Expr *compile_statement(Statement statement);
    S_Expr *compile_block(Block *block);
    S_Expr *compile_func_body(Block *block);
    S_Expr *compile_func_def(Func_Def func_def);
    S_Expr *compile_module(Module module);
};

#endif  // WCC_WAT_COMPILER_HPP_
