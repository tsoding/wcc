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
    End_Of_File,
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

Result alexer(String_View input, Dynamic_Array<Token> *tokens);

#endif  // WCC_ALEXER_HPP_
