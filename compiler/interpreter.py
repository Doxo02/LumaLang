from parser import *
import time

class LED:
    def fill_hsv(self, h, s, v): pass

class Interpreter:
    def __init__(self, prog: Program):
        self._prog = prog
        self.stack = []
        self.varStor = dict()

    def set_led(self, led: LED):
        self.led = led

    def eval_prog(self):
        for s in self._prog.statements:
            self.eval_stmt(s)

    def eval_stmt(self, stmt: Statement):
        if isinstance(stmt, IfElse):
            self.eval_expr(stmt.cond)
            res = self.stack.pop()
            if res == 1.0:
                self.eval_stmt(stmt.body)
            elif stmt.elseBody is not None:
                self.eval_stmt(stmt.elseBody)
        elif isinstance(stmt, ExprStatement):
            before_len = len(self.stack)
            self.eval_expr(stmt.expr)
            for i in range(len(self.stack) - before_len):
                self.stack.pop()
        elif isinstance(stmt, Loop):
            while True:
                self.eval_stmt(stmt.body)
        elif isinstance(stmt, Block):
            for s in stmt.stmts:
                self.eval_stmt(s)
        elif isinstance(stmt, VarDeclaration):
            res = 0.0
            if stmt.expr is not None:
                self.eval_expr(stmt.expr)
                res = self.stack.pop()
            self.varStor[stmt.id] = res
        elif isinstance(stmt, Function): pass

    def eval_expr(self, expr: Expression):
        if isinstance(expr, Assignment):
            self.eval_expr(expr.expr)
            res = self.stack.pop()
            self.varStor[expr.id] = res
        elif isinstance(expr, Binary):
            self.eval_expr(expr.lhs)
            self.eval_expr(expr.rhs)
            rhs = self.stack.pop()
            lhs = self.stack.pop()
            match expr.op:
                case BinOp.PLUS:
                    self.stack.append(lhs + rhs)
                case BinOp.MINUS:
                    self.stack.append(lhs - rhs)
                case BinOp.MULT:
                    self.stack.append(lhs * rhs)
                case BinOp.DIV:
                    self.stack.append(lhs / rhs)
                case BinOp.MOD:
                    self.stack.append(float(int(lhs) % int(rhs)))
                case BinOp.EQUALS:
                    self.stack.append(float(lhs == rhs))
                case BinOp.NEQUALS:
                    self.stack.append(float(lhs != rhs))
                case BinOp.LEQUALS:
                    self.stack.append(float(lhs <= rhs))
                case BinOp.GEQUALS:
                    self.stack.append(float(lhs >= rhs))
                case BinOp.LESS:
                    self.stack.append(float(lhs < rhs))
                case BinOp.GREATER:
                    self.stack.append(float(lhs > rhs))
                case BinOp.LAND:
                    if lhs == 1.0 and rhs == 1.0:
                        self.stack.append(1.0)
                    else:
                        self.stack.append(0.0)
                case BinOp.LOR:
                    if lhs == 1.0 or rhs == 1.0:
                        self.stack.append(1.0)
                    else:
                        self.stack.append(0.0)
        elif isinstance(expr, Call):
            args = []
            for a in expr.args:
                self.eval_expr(a)
                args.append(self.stack.pop())
            if expr.id == "fill_hsv":
                self.led.fill_hsv(args[0], args[1], args[2])
            elif expr.id == "delay":
                time.sleep(args[0] / 1000)
        elif isinstance(expr, Number):
            self.stack.append(expr.val)
        elif isinstance(expr, Var):
            self.stack.append(self.varStor[expr.id])