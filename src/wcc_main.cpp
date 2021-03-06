// TODO: generating wasm directly is not supported
// TODO: generating dot for ast is not supported
enum class Target
{
    Wat,
    Ast,
    Tokens,
};

void print1(FILE *stream, Target target)
{
    switch (target) {
    case Target::Wat: print1(stream, "wat"); break;
    case Target::Ast: print1(stream, "ast"); break;
    case Target::Tokens: print1(stream, "tokens"); break;
    }
}

Maybe<Target> target_by_name(String_View name)
{
    if (name == "wat"_sv)    return {true, Target::Wat};
    if (name == "ast"_sv)    return {true, Target::Ast};
    if (name == "tokens"_sv) return {true, Target::Tokens};
    return {};
}

void usage(FILE *stream)
{
    println(stream, "Usage: ./wcc [OPTIONS] <input.wc>");
    println(stream, "OPTIONS:");
    println(stream, "    --target|-t <target>     Generate code for a specific target.");
    println(stream, "                             Available targets: wat, ast, tokens.");
    println(stream, "                             Default target: wat.");
    println(stream, "    --help|-h                Print this help to stdout and exit with 0 exit code.");
    // TODO: output to a specific file is not supported (flag -o)
}

int main(int argc, char *argv[])
{
    auto target = Target::Wat;

    Args args = {argc, argv};
    args.shift();

    while (args.argc > 0) {
        String_View flag = cstr_as_string_view(*args.argv);

        if (!flag.has_prefix("-"_sv)) {
            break;
        }

        args.shift();

        if (flag == "--target"_sv || flag == "-t"_sv) {
            if (args.empty()) {
                usage(stderr);
                println(stderr, "ERROR: No value provided for flag `", flag, "`");
                exit(1);
            }

            auto custom_target_name = cstr_as_string_view(args.shift());
            auto custom_target = target_by_name(custom_target_name);
            if (!custom_target.has_value) {
                usage(stderr);
                println(stderr, "ERROR: Unknown target `", custom_target_name, "`");
                exit(1);
            }

            target = custom_target.unwrap;
        } else if (flag == "--help"_sv || flag == "-h"_sv) {
            usage(stdout);
            exit(0);
        } else {
            usage(stderr);
            println(stderr, "ERROR: Unknown flag `", flag, "`");
            exit(1);
        }
    }

    if (args.empty()) {
        usage(stderr);
        println(stderr, "ERROR: No input file provided");
        exit(1);
    }

    // TODO: Can we use Linear_Memory for read the input file instead of relying on malloc called by read_file_as_string_view()
    const char *input_filepath = args.shift();
    auto input = unwrap_or_panic(
        read_file_as_string_view(input_filepath),
        "Could not read file `", input_filepath, "`: ", strerror(errno));

    Linear_Memory memory = {};
    memory.capacity = 1024 * 1024 * 1024;
    memory.buffer = (uint8_t*) malloc(sizeof(uint8_t) * memory.capacity);

    Reporter reporter = {};
    reporter.filename = cstr_as_string_view(input_filepath);
    reporter.input = input;

    // TODO: Can we use Linear_Memory for collecting tokens instead of Dynamic_Array
    Alexer alexer = {};
    alexer.input = input;
    alexer.reporter = reporter;
    alexer.tokenize();

    if (target == Target::Tokens) {
        alexer.dump_tokens();
        return 0;
    }

    Parser parser = {};
    parser.memory = &memory;
    parser.reporter = reporter;
    parser.tokens = {alexer.tokens.size, alexer.tokens.data};
    Module module = parser.parse_module();
    assert(parser.tokens.count == 0 && "The tokens were not fully parsed");

    if (target == Target::Ast) {
        println(stdout, module);
        return 0;
    }

    Type_Checker type_checker = {};
    type_checker.memory = &memory;
    type_checker.reporter = reporter;
    type_checker.check_types_of_module(&module);

    Wat_Compiler wat_compiler = {};
    wat_compiler.memory = &memory;
    wat_compiler.reporter = reporter;

    S_Expr *wat = wat_compiler.compile_module(module);

    if (target == Target::Wat) {
        println(stdout, wat);
        return 0;
    }

    println(stdout, "Unsupported target `", target, "`");
    return 1;
}
