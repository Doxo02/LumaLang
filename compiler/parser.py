from tokenizer import TokType, Token, Tokenizer
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
        ret += "\n"
        ret += IDENT * ident_level
        ret += ")"
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
            ret += IDENT * ident_level
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
        ret += "\n"
        ret += IDENT * ident_level
        ret += ")"
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
        ret += IDENT * ident_level
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
            raise ParserError(f"Expected {type.name}, got {tok.to_string()}")
        return tok
    
    def accept(self, type: TokType) -> bool:
        if self.peek().type == type:
            self.next()
            return True
        return False

    def parse(self) -> Program: 
        tok = self.peek()
        stmts: list[Statement] = []
        while tok is not None and tok.type != TokType.EOF:
            stmts.append(self._parse_statement())
            tok = self.peek()
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

    def _parse_expression(self) -> Expression:
        return self._parse_assignment()

    def _parse_assignment(self) -> Expression:
        if self.peek().type == TokType.IDENTIFIER and self.peek(1).type == TokType.ASSIGN:
            id = self.expect(TokType.IDENTIFIER).val
            self.expect(TokType.ASSIGN)
            expr = self._parse_assignment()
            return Assignment(id, expr)
        return self._parse_logic_or()

    def _parse_logic_or(self) -> Expression:
        node = self._parse_logic_and()
        while self.accept(TokType.LOR):
            rhs = self._parse_logic_and()
            node = Binary(BinOp.LOR, node, rhs)
        return node

    def _parse_logic_and(self) -> Expression:
        node = self._parse_equality()
        while self.accept(TokType.LAND):
            rhs = self._parse_equality()
            node = Binary(BinOp.LAND, node, rhs)
        return node
    
    def _parse_equality(self) -> Expression:
        node = self._parse_comparison()
        while True:
            if self.accept(TokType.EQUALS):
                rhs = self._parse_comparison()
                node = Binary(BinOp.EQUALS, node, rhs)
            elif self.accept(TokType.NEQUALS):
                rhs = self._parse_comparison()
                node = Binary(BinOp.NEQUALS, node, rhs)
            else:
                break
        return node
    
    def _parse_comparison(self) -> Expression:
        node = self._parse_term()
        while True:
            if self.accept(TokType.GREATER):
                rhs = self._parse_term()
                node = Binary(BinOp.GREATER, node, rhs)
            elif self.accept(TokType.GEQUALS):
                rhs = self._parse_term()
                node = Binary(BinOp.GEQUALS, node, rhs)
            elif self.accept(BinOp.LESS):
                rhs = self._parse_term()
                node = Binary(BinOp.LESS, node, rhs)
            elif self.accept(BinOp.LEQUALS):
                rhs = self._parse_term()
                node = Binary(BinOp.LEQUALS, node, rhs)
            else:
                break
        return node

    def _parse_term(self) -> Expression:
        node = self._parse_factor()
        while True:
            if self.accept(TokType.PLUS):
                rhs = self._parse_factor()
                node = Binary(BinOp.PLUS, node, rhs)
            elif self.accept(TokType.MINUS):
                rhs = self._parse_factor()
                node = Binary(BinOp.MINUS, node, rhs)
            else:
                break
        return node
    
    def _parse_factor(self) -> Expression:
        node = self._parse_unary()
        while True:
            if self.accept(TokType.MULT):
                rhs = self._parse_unary()
                node = Binary(BinOp.MULT, node, rhs)
            elif self.accept(TokType.DIV):
                rhs = self._parse_unary()
                node = Binary(BinOp.DIV, node, rhs)
            elif self.accept(TokType.MOD):
                rhs = self._parse_unary()
                node = Binary(BinOp.MOD, node, rhs)
            else:
                break
        return node
    
    def _parse_unary(self) -> Expression:
        if self.accept(TokType.MINUS):
            rhs = self._parse_unary()
            return Binary(BinOp.MINUS, Number(0), rhs)
        return self._parse_call()

    def _parse_call(self) -> Expression:
        if self.peek().type == TokType.IDENTIFIER and self.peek(1).type == TokType.LPAREN:
            id = self.next().val
            self.expect(TokType.LPAREN)
            args: list[Expression] = []
            if not self.accept(TokType.RPAREN):
                args.append(self._parse_expression())
                while self.accept(TokType.COMMA):
                    args.append(self._parse_expression())
                self.expect(TokType.RPAREN)
            return Call(id, args)
        return self._parse_primary()

    def _parse_primary(self) -> Expression:
        tok = self.next()

        if tok.type == TokType.NUMBER:
            return Number(float(tok.val))
        elif tok.type == TokType.IDENTIFIER:
            return Var(tok.val)
        elif tok.type == TokType.LPAREN:
            expr = self._parse_expression()
            self.expect(TokType.RPAREN)
            return expr
        else:
            raise ParserError(f"Unexpected token: {tok.to_string()}")

