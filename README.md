# C Compiler

[Using the C11 standard](https://www.open-std.org/jtc1/sc22/WG14/www/docs/n1256.pdf)

It will probably be very simplified.

I hope to be able to bootstrap this compiler.

## To Do

- [ ] Lexer
    - [x] Character Literals
        - [x] Fix character escape bug. An escaped character hex or oct literal must have the exact length of a byte. A hex value must have two digits and an oct needs three. Anything shorter throws an error.
    - [x] String literals
        - [x] Implement processing escape chars for string literals
    - [ ] Identifiers
    - [ ] Integer constants
    - [ ] Floating constants
- [ ] Recursive Descent Parser

## Missing features

I may implement these in the future. For now they are unimportant and on the backburner.

- Preprocessor
- Punctuators such as `?`, and `...`. Also all compound assignment punctuators like `*=`.
- Multicharacter literals are unsupported.
- Character literal prefixes such as `u'\xFF'` are not supported currently. All char literals are considered signed.

Functions such as printf will be built in for debugging purposes and also linking with the actual c standard library would be impossible unless I worked on this for months straight.
