#include "tokenizer.h"
#include "error.h"
#include <iostream>

bool ConstantToken::operator==(const ConstantToken& other) const {
    return (value == other.value);
}

bool SymbolToken::operator==(const SymbolToken& other) const {
    return (name == other.name);
}

bool DotToken::operator==(const DotToken&) const {
    return true;
}

bool QuoteToken::operator==(const QuoteToken&) const {
    return true;
}

Tokenizer::Tokenizer(std::istream* in) : in_(in) {
    Next();
}

bool Tokenizer::IsEnd() {
    return flag_;
}

void Tokenizer::Next() {
    std::string first = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ<=>*/#";
    std::string second = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ<=>*/#0123456789?!-";
    while (!in_->eof() && std::isspace(in_->peek())) {
        in_->get();
    }
    if (in_->eof()) {
        flag_ = true;
        return;
    }
    int c = in_->get();
    switch (c) {
        case '(': token_ = Token{BracketToken::OPEN}; break;
        case ')': token_ = Token{BracketToken::CLOSE}; break;
        case '.': token_ = Token{DotToken{}}; break;
        case '\'': token_ = Token{QuoteToken{}}; break;
        case '-':
        case '+': {
            if (!std::isdigit(in_->peek())) {
                std::string temp;
                temp += static_cast<char>(c);
                token_ = Token{SymbolToken{temp}}; break;
            }
            int sgn = (c == '-') ? -1 : 1;
            *in_ >> c;
            token_ = Token{ConstantToken{sgn * c}}; break;
        }
        default: {
            if (std::isdigit(c)) {
                c = static_cast<int>(static_cast<char>(c) - '0');
                while (!in_->eof() && std::isdigit(in_->peek())) {
                    c *= 10;
                    c += static_cast<int>(static_cast<char>(in_->get()) - '0');
                }
                token_ = Token{ConstantToken{c}}; break;
            }
            if (first.find(static_cast<char>(c)) == std::string::npos) {
                throw SyntaxError("Error");
            }
            std::string s;
            s += static_cast<char>(c);
            while (!in_->eof() && second.find(in_->peek()) != std::string::npos) {
                s += in_->get();
            }
            token_ = Token{SymbolToken{s}}; break;
        }
    }
}

Token Tokenizer::GetToken() {
    return token_;
}