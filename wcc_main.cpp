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
    switch (expr->type) {
    case S_Expr_Type::Atom: {
        print(stream, expr->atom.name);
    } break;

    case S_Expr_Type::Cons: {
        print(stream, "(");
        while (expr && expr->type == S_Expr_Type::Cons) {
            print(stream, expr->cons.head, " ");
            expr = expr->cons.tail;
        }

        if (expr) {
            print(stream, " . ", expr);
        }

        print(stream, ")");
    } break;
    }
}

struct Wat_Compiler
{
    Memory memory;

    S_Expr *atom(String_View name)
    {
        S_Expr *expr = memory.alloc<S_Expr>();
        expr->type = S_Expr_Type::Atom;
        expr->atom.name = name;
        return expr;
    }

    S_Expr *cons(S_Expr *head, S_Expr *tail)
    {
        S_Expr *expr = memory.alloc<S_Expr>();
        expr->type = S_Expr_Type::Cons;
        expr->cons.head = head;
        expr->cons.tail = tail;
        return expr;
    }

    S_Expr *list(S_Expr *head)
    {
        return cons(head, nullptr);
    }

    template <typename... Tail>
    S_Expr *list(S_Expr *head, Tail... tail)
    {
        return cons(head, list(tail...));
    }

    S_Expr *compile_func_def(Func_Def func_def)
    {
        return list(atom("func"_sv), atom(func_def.name));
    }

    S_Expr *compile_module_to_wat(Module module)
    {
        S_Expr *result = nullptr;

        Top_Level_Def *top_level_def = module.top_level_defs;
        while (top_level_def) {
            result = cons(compile_func_def(top_level_def->func_def), result);
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

    const char *input_filepath = args.shift();
    auto input = unwrap_or_panic(
        read_file_as_string_view(input_filepath),
        "Could not read file `", input_filepath, "`: ", strerror(errno));

    Dynamic_Array<Token> tokens = {};
    auto result = alexer(input, &tokens);
    if (result.failed) {
        println(stderr, "Alexer failed at ", input_filepath, ":", result.position, ": ", result.message);
        exit(1);
    }

    Parser parser = {};
    parser.memory.capacity = 1024 * 1024 * 1024;
    parser.memory.buffer = (uint8_t*) malloc(sizeof(uint8_t) * parser.memory.capacity);
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
    // TODO: Parser and Compile should share Memory via ptr
    wat_compiler.memory = parser.memory;
    println(stdout, "Generated WAT:");
    println(stdout, "    ", wat_compiler.compile_module_to_wat(module));

    return 0;
}
