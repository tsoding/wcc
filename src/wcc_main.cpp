void usage(FILE *stream)
{
    println(stream, "Usage: ./wcc <input.wc>");
}

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
    wat_compiler.input = input;
    wat_compiler.filename = cstr_as_string_view(input_filepath);
    println(stdout, "Generated WAT:");
    println(stdout, "    ", wat_compiler.compile_module(module));

    println(stdout, "Used memory: ", wat_compiler.memory->size, " bytes");

    return 0;
}
