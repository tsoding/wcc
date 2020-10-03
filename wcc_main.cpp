using namespace aids;

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

    Func_Def func_def = parser.parse_func_def();

    println(stdout, "Parsed code: ", func_def);
    println(stdout, "Unparsed tokens: ");

    for (size_t i = 0; i < parser.tokens.count; ++i) {
        println(stdout, "  ", parser.tokens.items[i].type, " -> \"", Escape { parser.tokens.items[i].text }, "\"");
    }

    return 0;
}
