from tokenizer import TokType, Token
from enum import Enum

IDENT = "  "

class ASTNode:
    def to_string(self, ident_level = 0):
        ret = IDENT * ident_level
        ret += "ASTNode"
        return ret

class Statement(ASTNode):
    def to_string(self, ident_level = 0):
        ret = IDENT * ident_level
        ret += "Statement"
        return ret

class Expression(ASTNode):
    def to_string(self, ident_level = 0):
        ret = IDENT * ident_level
        ret += "Expression"
        return ret

class Assignment(Expression):
    def __init__(self, id: str, expr: Expression):
        self.id = id
        self.expr = expr
    
    def to_string(self, ident_level=0):
        ret = IDENT * ident_level
        ret += f"Assignment ({self.id}):\n"
        ret += self.expr.to_string(ident_level+1)
        return ret

class BinOp(Enum):
    PLUS = "+"
    MINUS = "-"
    MULT = "*"
    DIV = "/"
    MOD = "%"

    EQUALS = "=="
    NEQUALS = "!="
    LEQUALS = "<="
    GEQUALS = ">="
    LESS = "<"
    GREATER = ">"

    LAND = "and"
    LOR = "or"

class Binary(Expression):
    def __init__(self, op: BinOp, lhs: Expression, rhs: Expression):
        self.op = op
        self.lhs = lhs
        self.rhs = rhs
    
    def to_string(self, ident_level=0):
        ret = IDENT * ident_level
        ret += f"BinOp ({self.op.value}): (\n"
        ret += IDENT * (ident_level+1)
        ret += "Left:\n"
        ret += self.lhs.to_string(ident_level+2)
        ret += "\n"
        ret += IDENT * (ident_level+1)
        ret += "Right:\n"
        ret += self.rhs.to_string(ident_level+2)
        ret += "\n)"
        return ret

class Call(Expression):
    def __init__(self, id: str, args: list[Expression]):
        self.id = id
        self.args = args

    def to_string(self, ident_level=0):
        ret = IDENT * ident_level
        ret += f"Call ({self.id})"
        if (len(self.args) > 0):
            ret += ": (\n"
            for a in self.args:
                ret += a.to_string(ident_level+1)
                ret += ",\n"
            ret += ")"
        return ret

class Number(Expression):
    def __init__(self, val: float):
        self.val = val
    
    def to_string(self, ident_level=0):
        ret = IDENT * ident_level
        ret += f"Number: {self.val}"
        return ret

class Var(Expression):
    def __init__(self, id: str):
        self.id = id
    
    def to_string(self, ident_level=0):
        ret = IDENT * ident_level
        ret += f"Var: {self.id}"
        return ret

class ExprStatement(Statement):
    def __init__(self, expr: Expression):
        self.expr = expr
    
    def to_string(self, ident_level = 0):
        ret = IDENT * ident_level
        ret += "ExprStatement:\n"
        ret += self.expr.to_string(ident_level+1)
        return ret
    
class IfElse(Statement):
    def __init__(self, cond: Expression, body: Statement, elseBody: Statement | None = None):
        self.cond = cond
        self.body = body
        self.elseBody = elseBody

    def to_string(self, ident_level=0):
        ret = IDENT * ident_level
        ret += "If: (\n"
        ret += IDENT * (ident_level+1)
        ret += "condition:\n"
        ret += self.cond.to_string(ident_level+2)
        ret += "\n"
        ret += IDENT * (ident_level+1)
        ret += "body:\n"
        ret += self.body.to_string(ident_level+2)
        if self.elseBody is not None:
            ret += "\n"
            ret += IDENT * (ident_level+1)
            ret += "else body:\n"
            ret += self.elseBody.to_string(ident_level+2)
        ret += "\n)"
        return ret

class Loop(Statement):
    def __init__(self, body: Statement):
        self.body = body

    def to_string(self, ident_level=0):
        ret = IDENT * ident_level
        ret += "Loop:\n"
        ret += self.body.to_string(ident_level+2)
        return ret
    
class Block(Statement):
    def __init__(self, stmts: list[Statement]):
        self.stmts = stmts
    
    def to_string(self, ident_level=0):
        ret = IDENT * ident_level
        ret += "Block: (\n"
        for s in self.stmts:
            ret += s.to_string(ident_level+1)
            ret += "\n"
        ret += ")"
        return ret
    
class VarDeclaration(Statement):
    def __init__(self, id: str, expr: Expression | None = None):
        self.id = id
        self.expr = expr
    
    def to_string(self, ident_level=0):
        ret = IDENT * ident_level
        ret += f"VarDeclaration ({self.id})"
        if self.expr is not None:
            ret += ":\n"
            ret += self.expr.to_string(ident_level+1)
        return ret

class Function(Statement):
    def __init__(self, id: str, params: list[str], body: Statement):
        self.id = id
        self.params = params
        self.body = body
    
    def to_string(self, ident_level=0):
        ret = IDENT * ident_level
        ret += f"Function ({self.id}"
        if len(self.params) > 0:
            ret += "{"
            first = True
            for p in self.params:
                if first:
                    first = False
                else:
                    ret += ", "
                ret += p
            ret +="}"
        ret += "):\n"
        ret += self.body.to_string(ident_level+1)
        return ret
    
class Program(ASTNode):
    def __init__(self, statements: list[Statement]):
        self.statements = statements
    
    def to_string(self, ident_level=0):
        ret = IDENT * ident_level
        ret += "Program: (\n"
        for s in self.statements:
            ret += s.to_string(ident_level+1)
            ret += "\n"
        ret += IDENT * ident_level
        ret += ")"
        return ret

class ParserError(Exception): pass

class Parser:
    def __init__(self, tokens: list[Token]):
        self.tokens = tokens
        self.i = 0

    def peek(self, amount: int = 0) -> Token:
        return self.tokens[self.i + amount]
    
    def next(self) -> Token: 
        tok = self.tokens[self.i]
        self.i += 1
        return tok
    
    def expect(self, type: TokType) -> Token:
        tok = self.next()
        if tok.type != type:
            raise ParserError(f"Expected {type.name}, got {tok.type.name}")
        return tok
    
    def accept(self, type: TokType) -> bool:
        if self.peek().type == type:
            self.next()
            return True
        return False

    def parse(self) -> Program: 
        tok = self.next()
        stmts: list[Statement] = []
        while tok is not None and tok.type != TokType.EOF:
            stmts.append(self._parse_statement())
        return Program(stmts)

    def _parse_statement(self) -> Statement:
        match self.peek().type:
            case TokType.IF:
                return self._parse_if_else()
            case TokType.LOOP:
                return self._parse_loop()
            case TokType.LET:
                return self._parse_var_decl()
            case TokType.FN:
                return self._parse_fn_decl()
            case TokType.LBRACE:
                return self._parse_block()
        
        expr = self._parse_expression()
        self.expect(TokType.SEMICOLON)
        return ExprStatement(expr)

    def _parse_if_else(self) -> Statement:
        self.expect(TokType.IF)
        self.expect(TokType.LPAREN)
        cond = self._parse_expression()
        self.expect(TokType.RPAREN)
        body = self._parse_statement()
        elseBody = None
        if self.accept(TokType.ELSE):
            elseBody = self._parse_statement()
        return IfElse(cond, body, elseBody)

    def _parse_loop(self) -> Statement: 
        self.expect(TokType.LOOP)
        body = self._parse_statement()
        return Loop(body)
    
    def _parse_block(self) -> Statement:
        self.expect(TokType.LBRACE)
        stmts: list[Statement] = []
        while self.peek().type != TokType.RBRACE:
            stmts.append(self._parse_statement())
        self.expect(TokType.RBRACE)
        return Block(stmts)
    
    def _parse_var_decl(self) -> Statement:
        self.expect(TokType.LET)
        id = self.expect(TokType.IDENTIFIER).val
        expr = None
        if self.accept(TokType.ASSIGN):
            expr = self._parse_expression()
        self.expect(TokType.SEMICOLON)
        return VarDeclaration(id, expr)

    def _parse_fn_decl(self) -> Statement:
        self.expect(TokType.FN)
        id = self.expect(TokType.IDENTIFIER).val
        self.expect(TokType.LPAREN)
        params: list[str] = []
        if not self.accept(TokType.RPAREN):
            params.append(self.expect(TokType.IDENTIFIER).val)
            while self.accept(TokType.COMMA):
                params.append(self.expect(TokType.IDENTIFIER).val)
            self.expect(TokType.RPAREN)
        body = self._parse_statement()
        return Function(id, params, body)

    def _parse_expression(self) -> Expression: pass
    def _parse_assignment(self) -> Expression: pass
    def _parse_logic_or(self) -> Expression: pass
    def _parse_logic_and(self) -> Expression: pass
    def _parse_equality(self) -> Expression: pass
    def _parse_comparison(self) -> Expression: pass
    def _parse_term(self) -> Expression: pass
    def _parse_factor(self) -> Expression: pass
    def _parse_unary(self) -> Expression: pass
    def _parse_call(self) -> Expression: pass
    def _parse_primary(self) -> Expression: pass


program="""
let hue = 0;

loop {
    fill_hsv(hue, 255, 255); // Test
    hue = (hue + 1) % 255;
    delay(20);
}
"""