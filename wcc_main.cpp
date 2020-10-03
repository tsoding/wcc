using namespace aids;

void usage(FILE *stream)
{
    println(stream, "Usage: ./wcc <input.wc>");
}

struct Escape
{
    String_View unwrap;
};

void print1(FILE *stream, Escape escape)
{
    for (size_t i = 0; i < escape.unwrap.count; ++i) {
        switch (escape.unwrap.data[i]) {
        case '\a': print(stream, "\\a"); break;
        case '\b': print(stream, "\\b"); break;
        case '\f': print(stream, "\\f"); break;
        case '\n': print(stream, "\\n"); break;
        case '\r': print(stream, "\\r"); break;
        case '\t': print(stream, "\\t"); break;
        case '\v': print(stream, "\\v"); break;
        default: print(stream, escape.unwrap.data[i]);
        }
    }
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

    for (size_t i = 0; i < tokens.size; ++i) {
        println(stdout, tokens.data[i].type, " -> \"", Escape { tokens.data[i].text }, "\"");
    }

    return 0;
}
