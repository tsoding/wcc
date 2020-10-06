using namespace aids;

void usage(FILE *stream)
{
    println(stream, "Usage: ./wcc <input.wc>");
}

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

struct Wat_Compiler
{
    Linear_Memory *memory;

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

    S_Expr *wat_ident(String_View s)
    {
        return atom(concat("$"_sv, s));
    }

    String_View wat_name_of_type(Type type)
    {
        switch (type) {
        case Type::I32:
            return "i32"_sv;
            break;
        }
        return {};
    }

    S_Expr *compile_var_def(Var_Def var_def)
    {
        return list(atom("param"_sv),
                    wat_ident(var_def.name),
                    atom(wat_name_of_type(var_def.type)));
    }

    S_Expr *compile_args_list(Args_List *args_list)
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

    S_Expr *compile_return_type(Type type)
    {
        return list(atom("result"_sv), atom(wat_name_of_type(type)));
    }

    S_Expr *compile_number_literal(Number_Literal number_literal)
    {
        return list(atom("i32.const"_sv), atom(number_literal.unwrap));
    }

    S_Expr *compile_variable(Variable variable)
    {
        return nullptr;
    }

    S_Expr *compile_plus(Plus plus)
    {
        return nullptr;
    }

    S_Expr *compile_greater(Greater greater)
    {
        return nullptr;
    }

    S_Expr *compile_expression(Expression *expression)
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

        return list(atom("i32.const"_sv), atom("69"_sv));
    }

    S_Expr *compile_func_body(Block *block)
    {
        S_Expr *local_var_def_section = nullptr;
        S_Expr *local_var_def_init = nullptr;

        while (block && block->statement.type == Statement_Type::Local_Var_Def) {
            auto var_ident = wat_ident(block->statement.local_var_def.def.name);

            local_var_def_section = append(
                local_var_def_section,
                list(list(atom("local"_sv),
                          var_ident,
                          atom(wat_name_of_type(block->statement.local_var_def.def.type)))));

            local_var_def_init = append(
                local_var_def_init,
                list(list(atom("set_local"_sv),
                          var_ident,
                          compile_expression(block->statement.local_var_def.value))));

            block = block->next;
        }

        return append(local_var_def_section, local_var_def_init);
    }

    S_Expr *compile_func_def(Func_Def func_def)
    {
        return append(
            list(atom("func"_sv),
                 atom(concat("$"_sv, func_def.name)),
                 list(atom("export"_sv), atom(concat("\""_sv, func_def.name, "\""_sv)))),
            compile_args_list(func_def.args_list),
            list(compile_return_type(func_def.return_type)),
            compile_func_body(func_def.body));
    }

    S_Expr *compile_module_to_wat(Module module)
    {
        S_Expr *result = nullptr;

        Top_Level_Def *top_level_def = module.top_level_defs;
        while (top_level_def) {
            result = append(result, list(compile_func_def(top_level_def->func_def)));
            top_level_def = top_level_def->next;
        }

        return cons(atom("module"_sv), result);
    }
};

int main(int argc, char *argv[])
{
    Args args = {argc, argv};
    args.shift();

    if (args.empty()) {
        println(stderr, "ERROR: No input file provided");
        usage(stderr);
        exit(1);
    }

    // TODO: Can we use Linear_Memory for read the input file instead of relying on malloc called by read_file_as_string_view()
    const char *input_filepath = args.shift();
    auto input = unwrap_or_panic(
        read_file_as_string_view(input_filepath),
        "Could not read file `", input_filepath, "`: ", strerror(errno));

    // TODO: Can we use Linear_Memory for collecting tokens instead of Dynamic_Array
    Dynamic_Array<Token> tokens = {};
    auto result = alexer(input, &tokens);
    if (result.failed) {
        println(stderr, "Alexer failed at ", input_filepath, ":", result.position, ": ", result.message);
        exit(1);
    }

    Linear_Memory memory = {};
    memory.capacity = 1024 * 1024 * 1024;
    memory.buffer = (uint8_t*) malloc(sizeof(uint8_t) * memory.capacity);

    Parser parser = {};
    parser.memory = &memory;
    parser.tokens = {tokens.size, tokens.data};
    parser.input = input;
    parser.filename = cstr_as_string_view(input_filepath);

    Module module = parser.parse_module();
    println(stdout, "Parsed code:");
    println(stdout, "    ", module);

    if (parser.tokens.count > 0) {
        parser.fail("The tokens were not fully parsed");
    }

    Wat_Compiler wat_compiler = {};
    wat_compiler.memory = &memory;
    println(stdout, "Generated WAT:");
    println(stdout, "    ", wat_compiler.compile_module_to_wat(module));

    println(stdout, "Used memory: ", wat_compiler.memory->size, " bytes");

    return 0;
}
