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

    Module module = parser.parse_module();

    println(stdout, "Parsed code: ", module);

    if (parser.tokens.count > 0) {
        parser.fail("The tokens were not fully parsed");
    }


    return 0;
}
