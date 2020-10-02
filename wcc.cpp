#include <assert.h>
#include <stdio.h>
#include "aids.hpp"

using namespace aids;

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

void print1(FILE *stream, Token_Type type)
{
    switch (type) {
    case Token_Type::Func: print(stream, "Func"); break;
    case Token_Type::Symbol: print(stream, "Symbol"); break;
    case Token_Type::Open_Paren: print(stream, "Open_Paren"); break;
    case Token_Type::Closed_Paren: print(stream, "Closed_Paren"); break;
    case Token_Type::Colon: print(stream, "Colon"); break;
    case Token_Type::Open_Curly: print(stream, "Open_Curly"); break;
    case Token_Type::Closed_Curly: print(stream, "Closed_Curly"); break;
    case Token_Type::Local: print(stream, "Local"); break;
    case Token_Type::Equals: print(stream, "Equals"); break;
    case Token_Type::Number_Literal: print(stream, "Number_Literal"); break;
    case Token_Type::While: print(stream, "While"); break;
    case Token_Type::Greater: print(stream, "Greater"); break;
    case Token_Type::Plus: print(stream, "Plus"); break;
    case Token_Type::Minus_Equals: print(stream, "Minus_Equals"); break;
    case Token_Type::Minus: print(stream, "Minus"); break;
    case Token_Type::Newline: print(stream, "Newline"); break;
    }
}

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

String_View trim_tabs_and_spaces(String_View s)
{
    while (s.count > 0 && (*s.data == ' ' || *s.data == '\t')) {
        s.chop(1);
    }
    return s;
}

template <typename Predicate>
String_View chop_while(String_View *s, Predicate predicate)
{
    size_t n = 0;
    while (n < s->count && predicate(s->data[n])) {
        n += 1;
    }

    String_View chunk = s->subview(0, n);
    s->chop(n);

    return chunk;
}

String_View chop_off(String_View *s, size_t n)
{
    auto result = s->subview(0, n);
    s->chop(n);
    return result;
}

Result alexer(String_View input, Dynamic_Array<Token> *tokens)
{
    auto source = trim_tabs_and_spaces(input);
    while (source.count > 0) {
        if (isalpha(*source.data)) {
            String_View token_text = chop_while(&source, isalnum);

            if (token_text == "while"_sv) {
                tokens->push(Token {Token_Type::While, token_text});
            } else if (token_text == "func"_sv) {
                tokens->push(Token {Token_Type::Func, token_text});
            } else if (token_text == "local"_sv) {
                tokens->push(Token {Token_Type::Local, token_text});
            } else {
                tokens->push(Token {Token_Type::Symbol, token_text});
            }
        } else if (isdigit(*source.data)) {
            String_View token_text = chop_while(&source, isdigit);
            tokens->push(Token {Token_Type::Number_Literal, token_text});
        } else {
            switch (*source.data) {
            case '(':
                tokens->push(Token {Token_Type::Open_Paren, chop_off(&source, 1)});
                break;
            case ')':
                tokens->push(Token {Token_Type::Closed_Paren, chop_off(&source, 1)});
                break;
            case ':':
                tokens->push(Token {Token_Type::Colon, chop_off(&source, 1)});
                break;
            case '{':
                tokens->push(Token {Token_Type::Open_Curly, chop_off(&source, 1)});
                break;
            case '}':
                tokens->push(Token {Token_Type::Closed_Curly, chop_off(&source, 1)});
                break;
            case '=':
                tokens->push(Token {Token_Type::Equals, chop_off(&source, 1)});
                break;
            case '>':
                tokens->push(Token {Token_Type::Greater, chop_off(&source, 1)});
                break;
            case '+':
                tokens->push(Token {Token_Type::Plus, chop_off(&source, 1)});
                break;
            case '-':
                if (source.count > 1 && source.data[1] == '=') {
                    tokens->push(Token {Token_Type::Minus_Equals, chop_off(&source, 2)});
                } else {
                    tokens->push(Token {Token_Type::Minus, chop_off(&source, 1)});
                }
                break;
            case '\n':
                tokens->push(Token {Token_Type::Newline, chop_off(&source, 1)});
                break;

            default: {
                assert(input.data <= source.data);
                return {true, static_cast<size_t>(source.data - input.data), "Unexpected character"};
            }
            }
        }
        source = trim_tabs_and_spaces(source);
    }

    return {};
}

template <typename T, typename... Args>
T unwrap_or_panic(Maybe<T> maybe, Args... args)
{
    if (!maybe.has_value) {
        println(stderr, args...);
        exit(1);
    }

    return maybe.unwrap;
}

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
    args.pop();

    if (args.empty()) {
        println(stderr, "ERROR: No input file provided");
        usage(stderr);
        exit(1);
    }

    const char *input_filepath = args.pop();
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
