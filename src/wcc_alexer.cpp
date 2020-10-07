#include "./wcc_alexer.hpp"

bool is_semicolon(Token token)
{
    return Token_Type::Semicolon == token.type;
}

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
    case Token_Type::Semicolon: print(stream, "Semicolon"); break;
    case Token_Type::Comma: print(stream, "Comma"); break;
    }
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

bool is_symbol_start(char x)
{
    return isalpha(x) || x == '_';
}

bool is_symbol(char x)
{
    return isalnum(x) || x == '_';
}

void Alexer::tokenize()
{
    auto source = input.trim_begin();
    while (source.count > 0) {
        if (is_symbol_start(*source.data)) {
            String_View token_text = chop_while(&source, is_symbol);

            if (token_text == "while"_sv) {
                tokens.push(Token {Token_Type::While, token_text});
            } else if (token_text == "func"_sv) {
                tokens.push(Token {Token_Type::Func, token_text});
            } else if (token_text == "local"_sv) {
                tokens.push(Token {Token_Type::Local, token_text});
            } else {
                tokens.push(Token {Token_Type::Symbol, token_text});
            }
        } else if (isdigit(*source.data)) {
            String_View token_text = chop_while(&source, isdigit);
            tokens.push(Token {Token_Type::Number_Literal, token_text});
        } else {
            switch (*source.data) {
            case '(':
                tokens.push(Token {Token_Type::Open_Paren, chop_off(&source, 1)});
                break;
            case ')':
                tokens.push(Token {Token_Type::Closed_Paren, chop_off(&source, 1)});
                break;
            case ':':
                tokens.push(Token {Token_Type::Colon, chop_off(&source, 1)});
                break;
            case '{':
                tokens.push(Token {Token_Type::Open_Curly, chop_off(&source, 1)});
                break;
            case '}':
                tokens.push(Token {Token_Type::Closed_Curly, chop_off(&source, 1)});
                break;
            case '=':
                tokens.push(Token {Token_Type::Equals, chop_off(&source, 1)});
                break;
            case '>':
                tokens.push(Token {Token_Type::Greater, chop_off(&source, 1)});
                break;
            case ',':
                tokens.push(Token {Token_Type::Comma, chop_off(&source, 1)});
                break;
            case '+':
                tokens.push(Token {Token_Type::Plus, chop_off(&source, 1)});
                break;
            case '-':
                if (source.count > 1 && source.data[1] == '=') {
                    tokens.push(Token {Token_Type::Minus_Equals, chop_off(&source, 2)});
                } else {
                    tokens.push(Token {Token_Type::Minus, chop_off(&source, 1)});
                }
                break;
            case ';':
                tokens.push(Token {Token_Type::Semicolon, chop_off(&source, 1)});
                break;

            default: {
                assert(input.data <= source.data);
                fail(static_cast<size_t>(source.data - input.data), "Unexpected character `", *source.data, "`");
            }
            }
        }
        source = source.trim_begin();
    }
}

void Alexer::dump_tokens()
{
    for (size_t i = 0; i < tokens.size; ++i) {
        println(stdout, "  ", tokens.data[i].type, " -> \"", Escape { tokens.data[i].text }, "\"");
    }
}
