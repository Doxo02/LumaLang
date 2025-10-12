# Specification of LumaLang

## Grammar

```ebnf
program         = requirement* statement* EOF ;
requirement     = "require" IDENTIFIER ";" ;
statement       = exprStmt | declaration | ifStmt | loopStmt | block | returnStmt ;
returnStmt      = "return" expression ";" ;
ifStmt          = "if" "(" expression ")" statement ("else" statement)? ;
loopStmt        = "loop" statement ;
block           = "{" statement* "}" ;
exprStmt        = expression ";" ;
declaration     = varDecl | fnDecl ;
varDecl         = "let" IDENTIFIER ("=" expression)? ";" ;
fnDecl          = "fn" IDENTIFIER "(" parameters? ")" statement ;
parameters      = IDENTIFIER ("," IDENTIFIER)* ;
expression      = assignment ;
assignment      = IDENTIFIER "=" assignment | logic_or ;
logic_or        = logic_and ("or" logic_and)* ;
logic_and       = equality ("and" equality)* ;
equality        = comparison (("==" | "!=") comparison)* ;
comparison      = term ((">" | ">=" | "<" | "<=") term)* ;
term            = factor (("+" | "-") factor)* ;
factor          = unary (("*" | "/" | "%") unary)* ;
unary           = ("!" | "-") unary | call ;
argList         = expression ("," expression)* ;
call            = IDENTIFIER "(" argList? ")" | primary ;
primary         = NUMBER | IDENTIFIER | "(" expression ")" ;

IDENTIFIER      = (LETTER | "_") (LETTER | DIGIT | "_")* ;
NUMBER          = DIGIT* ;
```

