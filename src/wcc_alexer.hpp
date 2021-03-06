#ifndef WCC_ALEXER_HPP_
#define WCC_ALEXER_HPP_

#include "./wcc_reporter.hpp"

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
    Equals_Equals,
    Number_Literal,
    While,
    Greater,
    Less_Equals,
    Plus,
    Minus_Equals,
    Plus_Equals,
    Minus,
    Semicolon,
    And,
    Rem,
    If,
    Else,
    Asterisk,
    Return,
    True,
    False,
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

struct Alexer
{
    String_View input;
    Dynamic_Array<Token> tokens;
    Reporter reporter;

    void dump_tokens() const;
    void tokenize();
};

#endif  // WCC_ALEXER_HPP_
