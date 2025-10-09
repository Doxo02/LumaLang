from enum import Enum

class TokenizeError(Exception): pass

class TokType(Enum):
    PLUS = "+"
    MINUS = "-"
    MULT = "*"
    DIV = "/"
    MOD = "%"
    EQUALS = "=="
    NEQUALS = "!="
    GREATER = ">"
    LESS = "<"
    GEQUALS = ">="
    LEQUALS = "<="
    NOT = "!"

    LAND = "and"
    LOR = "or"

    COMMA = ","
    SEMICOLON = ";"
    ASSIGN = "="
    LBRACE = "{"
    RBRACE = "}"
    LPAREN = "("
    RPAREN = ")"

    IF = "if"
    ELSE = "else"
    LOOP = "loop"
    LET = "let"
    FN = "fn"

    IDENTIFIER = "id"
    NUMBER = "num"
    EOF = "eof"

class Token:
    def __init__(self, type: TokType, line: int, col: int, length: int = 1, val: str | None = None):
        self.type = type
        self.line = line
        self.col = col
        self.length = length
        self.val = val

    def to_string(self) -> str:
        ret = self.type.name
        ret += f"({self.line}:{self.col}"
        if self.length > 1:
            ret += f"-{self.col+self.length-1}"
        ret += ")"
        if self.val is not None:
            ret += f": {self.val}"
        return ret

class Tokenizer:
    def __init__(self, src: str):
        self.src = src
        self.line = 0
        self.col = 0
        self.i = 0
    
    def get_next(self) -> Token:
        # skip whitespaces
        while self.i < len(self.src) and self.src[self.i] in [" ", "\t", "\n"]:
            if self.src[self.i] == "\n":
                self.line += 1
                self.col = 0
            else:
                self.col += 1
            self.i += 1

        if self.i >= len(self.src):
            return Token(TokType.EOF, self.line, self.col)

        # check for Identifier or Keyword
        if self.src[self.i].isalpha() or self.src[self.i] == "_":
            val = self.src[self.i]
            self.i += 1
            while self.src[self.i].isalpha() or self.src[self.i] == "_":
                val += self.src[self.i]
                self.i += 1

            tok = None
            if val == "if":
                tok = Token(TokType.IF, self.line, self.col, len(val))
            elif val == "else":
                tok = Token(TokType.ELSE, self.line, self.col, len(val))
            elif val == "loop":
                tok = Token(TokType.LOOP, self.line, self.col, len(val))
            elif val == "let":
                tok = Token(TokType.LET, self.line, self.col, len(val))
            elif val == "fn":
                tok = Token(TokType.FN, self.line, self.col, len(val))
            elif val == "and":
                tok = Token(TokType.LAND, self.line, self.col, len(val))
            elif val == "or":
                tok = Token(TokType.LOR, self.line, self.col, len(val))
            
            if tok is None:
                tok = Token(TokType.IDENTIFIER, self.line, self.col, len(val), val)
            
            # print(tok.to_string())
            self.col += len(val)
            return tok
        
        # check for Number
        if self.src[self.i].isdigit():
            val = self.src[self.i]
            self.i += 1
            found_dot = False
            while self.src[self.i].isdigit() or self.src[self.i] == '.':
                if self.src[self.i] == '.':
                    if found_dot:
                        raise TokenizeError(f"Found second decimal seperator at: {self.line}:{self.col}")
                    found_dot = True
                val += self.src[self.i]
                self.i += 1
            tok = Token(TokType.NUMBER, self.line, self.col, len(val), val)
            self.col += len(val)
            return tok

        tok = None
        match self.src[self.i]:
            case '+':
                tok = Token(TokType.PLUS, self.line, self.col)
            case '-':
                tok = Token(TokType.MINUS, self.line, self.col)
            case '*':
                tok = Token(TokType.MULT, self.line, self.col)
            case '%':
                tok = Token(TokType.MOD, self.line, self.col)
            case ',':
                tok = Token(TokType.COMMA, self.line, self.col)
            case ';':
                tok = Token(TokType.SEMICOLON, self.line, self.col)
            case '{':
                tok = Token(TokType.LBRACE, self.line, self.col)
            case '}':
                tok = Token(TokType.RBRACE, self.line, self.col)
            case '(':
                tok = Token(TokType.LPAREN, self.line, self.col)
            case ')':
                tok = Token(TokType.RPAREN, self.line, self.col)
            case '>':
                if self.src[self.i+1] == '=':
                    tok = Token(TokType.GEQUALS, self.line, self.col, 2)
                    self.i += 1
                    self.col += 1
                else:
                    tok = Token(TokType.GREATER, self.line, self.col)
            case '<':
                if self.src[self.i+1] == '=':
                    tok = Token(TokType.LEQUALS, self.line, self.col, 2)
                    self.i += 1
                    self.col += 1
                else:
                    tok = Token(TokType.LESS, self.line, self.col)
            case '!':
                if self.src[self.i+1] == '=':
                    tok = Token(TokType.NEQUALS, self.line, self.col, 2)
                    self.i += 1
                    self.col += 1
                else:
                    tok = Token(TokType.NOT, self.line, self.col)
            case '=':
                if self.src[self.i+1] == '=':
                    tok = Token(TokType.EQUALS, self.line, self.col, 2)
                    self.i += 1
                    self.col += 1
                else:
                    tok = Token(TokType.ASSIGN, self.line, self.col)
            case '/':
                if self.src[self.i+1] == '/':
                    self.i += 1
                    while self.src[self.i] != '\n':
                        self.i += 1
                    self.i += 1
                    self.line += 1
                    self.col = 0
                    return self.get_next()
                else:
                    tok = Token(TokType.DIV, self.line, self.col)
            case None:
                tok = Token(TokType.EOF, self.line, self.col)
            case _:
                raise TokenizeError(f"Unexpected symbol: '{self.src[self.i]}', at: {self.line}:{self.col}")
        
        self.i += 1
        self.col += 1
        return tok

    def tokenize_all(self) -> list[Token]:
        toks = []
        cur = self.get_next()

        while cur.type != TokType.EOF:
            toks.append(cur)
            cur = self.get_next()
        toks.append(cur)

        return toks


def print_tokens(tokens: list[Token]):
    for t in tokens:
        print(t.to_string())