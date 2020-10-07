#ifndef WCC_ALEXER_HPP_
#define WCC_ALEXER_HPP_

enum class Token_Type
{
    Func,
    Symbol,
    Open_Paren,
    Closed_Paren,
    Colon,
    Comma,
    Open_Curly,
    Closed_Curly,
    Local,
    Equals,
    Number_Literal,
    While,
    Greater,
    Plus,
    Minus_Equals,
    Minus,
    Semicolon,
};

void print1(FILE *stream, Token_Type type);

struct Token
{
    Token_Type type;
    String_View text;
};

bool is_semicolon(Token type);

struct Result
{
    bool failed;
    size_t position;
    const char *message;
};

// TODO: alexer does not support comments

struct Alexer
{
    String_View input;
    String_View filename;
    Dynamic_Array<Token> tokens;

    template <typename... Args>
    void inform(size_t offset, Args... args)
    {
        String_View input0 = input;
        for (size_t line_number = 1; input0.count > 0; ++line_number) {
            String_View line = input0.chop_by_delim('\n');

            if (offset <= line.count) {
                println(stderr, filename, ":", line_number, ":", offset + 1, ": ", args...);
                break;
            }

            offset -= line.count + 1;
        }
    }

    template <typename... Args>
    void fail(size_t offset, Args... args)
    {
        inform(offset, "error: ", args...);
        exit(1);
    }

    void dump_tokens();
    void tokenize();
};

#endif  // WCC_ALEXER_HPP_
