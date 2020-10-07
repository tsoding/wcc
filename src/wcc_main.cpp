// TODO: generating wasm directly is not supported
enum class Target
{
    Wat,
    Ast,
    Tokens,
};

Maybe<Target> target_by_name(String_View name)
{
    if (name == "wat"_sv)    return {true, Target::Wat};
    if (name == "ast"_sv)    return {true, Target::Ast};
    if (name == "tokens"_sv) return {true, Target::Tokens};
    return {};
}

void usage(FILE *stream)
{
    println(stream, "Usage: ./wcc [--target|-t <target>] <input.wc>");
    println(stream, "    --target|-t <target>     Generate code for a specific target.");
    println(stream, "                             Available targets: wat, ast, tokens.");
    println(stream, "                             Default target: wat.");
    // TODO: --help flag is not supported
    // TODO: output to a specific file is not supported (flag -o)
}

int main(int argc, char *argv[])
{
    auto target = Target::Wat;

    Args args = {argc, argv};
    args.shift();

    while (args.argc > 1) {
        String_View flag = cstr_as_string_view(args.shift());

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

    // TODO: Can we use Linear_Memory for collecting tokens instead of Dynamic_Array
    Alexer alexer = {};
    alexer.input = input;
    alexer.filename = cstr_as_string_view(input_filepath);
    alexer.tokenize();

    Linear_Memory memory = {};
    memory.capacity = 1024 * 1024 * 1024;
    memory.buffer = (uint8_t*) malloc(sizeof(uint8_t) * memory.capacity);

    Parser parser = {};
    parser.memory = &memory;
    parser.tokens = {alexer.tokens.size, alexer.tokens.data};
    parser.input = input;
    parser.filename = cstr_as_string_view(input_filepath);

    Module module = parser.parse_module();

    assert(parser.tokens.count == 0 && "The tokens were not fully parsed");

    Wat_Compiler wat_compiler = {};
    wat_compiler.memory = &memory;
    wat_compiler.input = input;
    wat_compiler.filename = cstr_as_string_view(input_filepath);

    S_Expr *wat = wat_compiler.compile_module(module);

    switch (target) {
    case Target::Wat:
        println(stdout, wat);
        break;
    case Target::Ast:
        println(stdout, module);
        break;
    case Target::Tokens:
        alexer.dump_tokens();
        break;
    }

    return 0;
}
