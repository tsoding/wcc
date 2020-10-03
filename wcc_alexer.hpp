#ifndef WCC_ALEXER_HPP_
#define WCC_ALEXER_HPP_

enum class Token_Type
{
    Func,
    Symbol,
    Open_Paren,
    Closed_Paren,
    Colon,
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
    Newline,
};

void print1(FILE *stream, Token_Type type);

struct Token
{
    Token_Type type;
    String_View text;
};

struct Result
{
    bool failed;
    size_t position;
    const char *message;
};

Result alexer(String_View input, Dynamic_Array<Token> *tokens);

#endif  // WCC_ALEXER_HPP_
