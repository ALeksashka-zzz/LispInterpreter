#include "parser.h"
#include <iostream>

std::shared_ptr<Object> Read(Tokenizer* tokenizer) {
    if (tokenizer->IsEnd() || tokenizer->GetToken() == Token{BracketToken::CLOSE}
        || tokenizer->GetToken() == Token{DotToken()}) {
        throw SyntaxError("SyntaxError");
    }
    if (tokenizer->GetToken() == Token{BracketToken::OPEN}) {
        tokenizer->Next();
        if (tokenizer->GetToken() == Token{SymbolToken{"quote"}}) {
            std::shared_ptr<Symbol> first(new Symbol("quote"));
            if (tokenizer->IsEnd()) {
                throw SyntaxError("SyntaxError");
            }
            tokenizer->Next();
            auto second = Read(tokenizer);
            tokenizer->Next();
            return std::shared_ptr<Cell>(new Cell(first, second));
        }
        return ReadList(tokenizer);
    }
    if (tokenizer->GetToken() == Token{QuoteToken()}) {
        std::shared_ptr<Symbol> first(new Symbol("quote"));
        if (tokenizer->IsEnd()) {
            throw SyntaxError("SyntaxError");
        }
        tokenizer->Next();
        auto second = Read(tokenizer);
        return std::shared_ptr<Cell>(new Cell(first, second));
    }
    if (std::holds_alternative<SymbolToken>(tokenizer->GetToken())) {
        SymbolToken real_token = std::get<SymbolToken>(tokenizer->GetToken());
        tokenizer->Next();
        if (real_token.name == "#f" || real_token.name == "#t") {
            return std::shared_ptr<Boolean>(new Boolean(real_token.name));
        } else {
            return std::shared_ptr<Symbol>(new Symbol(real_token.name));
        }
    }
    ConstantToken real_token = std::get<ConstantToken>(tokenizer->GetToken());
    std::shared_ptr<Number> num(new Number(real_token.value));
    tokenizer->Next();
    return num;
}

std::shared_ptr<Object> ReadList(Tokenizer* tokenizer) {
    if (tokenizer->IsEnd() || tokenizer->GetToken() == Token{DotToken()}) {
        throw SyntaxError("SyntaxError");
    }
    if (tokenizer->GetToken() == Token{BracketToken::CLOSE}) {
        tokenizer->Next();
        return nullptr;
    }
    auto first = Read(tokenizer);
    if (tokenizer->IsEnd()) {
        throw SyntaxError("SyntaxError");
    }
    if (tokenizer->GetToken() == Token{BracketToken::CLOSE}) {
        tokenizer->Next();
        return std::shared_ptr<Cell>(new Cell(first, nullptr));
    }
    if (tokenizer->GetToken() == Token{DotToken{}}) {
        tokenizer->Next();
        auto second = Read(tokenizer);
        if (tokenizer->IsEnd() || !(tokenizer->GetToken() == Token{BracketToken::CLOSE})) {
            throw SyntaxError("SyntaxError");
        }
        tokenizer->Next();
        return std::shared_ptr<Cell>(new Cell(first, second));
    }
    return std::shared_ptr<Cell>(new Cell(first, ReadList(tokenizer)));
}
