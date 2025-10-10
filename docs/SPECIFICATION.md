# Specification of LumaLang

## Grammar

```ebnf
program         = statement* EOF ;
statement       = exprStmt | declaration | ifStmt | loopStmt | block ;
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
NUMBER          = DIGIT* "." DIGIT* ;
```

## Bytecode

| name  | hex  | description                                                            |
| :---- | :--- | :--------------------------------------------------------------------- |
| NOOP  | 00   | N/A                                                                    |
| PUSHI | 01   | push next 32 bits as int                                               |
| PUSHF | 02   | push next 32 bits as float                                             |
| PUSHB | 03   | push next 8 bits as bool                                               |
| ADD   | 10   | adds the top two values on the stack and pushes the result             |
| SUB   | 11   | subtracts the top two values on the stack and pushes the result        |
| MUL   | 12   | multiplys the top two values on the stack and pushes the result        |
| DIV   | 13   | divides the top two values on the stack and pushes the result          |
| MOD   | 14   | divides the top two values on the stack and pushes the remainder       |
| BAND  | 15   | bitwise and the top two values on the stack and pushes the result      |
| BOR   | 16   | bitwise or the top two values on the stack and pushes the result       |
| EQ    | 17   | pushes 1 on the stack if the top two values are equal                  |
| NEQ   | 18   | pushes 1 on the stack if the top two values ar not equal               |
| LEQ   | 19   | pushes 1 if stack-1 <= stack                                           |
| GEQ   | 1A   | pushes 1 if stack-1 >= stack                                           |
| LT    | 1B   | pushes 1 if stack-1 < stack                                            |
| GT    | 1C   | pushes 1 if stack-1 > stack                                            |
| JMP   | 20   | jumps to the instruction given by the next two bytes                   |
| JZ    | 21   | jumps to the instruction given by the next two bytes if stack top == 0 |