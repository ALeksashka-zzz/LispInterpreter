# Lisp Interpreter

Implemented interpreter for Scheme language.

Short summary:

**tokenizer:** class for converting an input stream of characters into a sequence of tokens (Number, Boolean, Bracket, Quote, Dot, Symbol).

**parser:** a set of methods that reads the token stream and builds a syntax tree based on them.

**scheme:** calculates the simple expressions (without variables and lambda functions)
