## CIBL Interpreter in Python
## process: program -> PARSER -> abstract syntax tree -> EVAL -> result


#### import modules

from __future__ import division
import math
import operator as op

#### Types

Symbol = str
List = list
Number = (float, int)

#### Parsing : parser, tokenizer, reading from tokens

def parse(program):
    """Parse CIBL expression from program.

    Keyword arguments:
    program -- str from which the expression is parsed.
    """
    return parse_tokens(tokenize(program))

def tokenize(program):
    """Converts a CIBL expression string into a list of tokens

    Keyword arguments
    program -- str from which the expression is tokenized
    """
    return program.replace("(", " ( ").replace(")", " ) ").split()

def parse_tokens(tokens):
    """Parse list of program tokens

    Keyword arguments:
    tokens -- list of program tokens of a Scheme expression
    """
    if len(tokens) == 0:
        # Parsing an empty list
        raise SyntaxError("Unexpected EOF while reading")
    token = tokens.pop(0)
    if token == "(":
        # Parse the list of tokens for any sub-expressions
        items = []
        while tokens[0] != ")":
            sub_items = parse_tokens(tokens)
            items.append(sub_items)
        # All parenthesis have been removed from items
        tokens.pop(0)
        return items
    elif token == ")":
        # Left parenthesis appeared without corresponding '('
        raise SyntaxError("Unexpected symbol ')'")
    else:
        # Atomic expressions are all that's left to parse
        return atom(token)

def atom(token):
    """Depending on the nature of token, return an atom of CIBL type Number or
    Symbol.

    Keyword Arguments:
    token -- String to parsed into an atomic expression
    """
    try:
        # Determine if token can be parsed as an int
        return int(token)
    except ValueError:
        try:
            # Determine if token can be parsed as a float
            return float(token)
        except ValueError:
            # token must be a Scheme Symbol rather than a number
            return Symbol(token)

#### Environments

def standard_env():
    """Environment for standard Scheme procedures
    Users can define their own procedures with the following expression:
    (define <symbol> <value>)

    Attributes:
        hi
    """
    env = Env()
    env.update(vars(math))
    env.update({
        '+':op.add, '-':op.sub, '*':op.mul, '/':op.truediv,
        '>':op.gt, '<':op.lt, '>=':op.ge, '<=':op.le, '=':op.eq,
        'abs':     abs,
        'append':  op.add,
        'begin':   lambda *x: x[-1],
        'car':     lambda x: x[0],
        'cdr':     lambda x: x[1:],
        'cons':    lambda x,y: [x] + y,
        'eq?':     op.is_,
        'equal?':  op.eq,
        'length':  len,
        'list':    lambda *x: list(x),
        'list?':   lambda x: isinstance(x,list),
        'map':     map,
        'max':     max,
        'min':     min,
        'not':     op.not_,
        'null?':   lambda x: x == [],
        'number?': lambda x: isinstance(x, Number),
        'procedure?': callable,
        'round':   round,
        'symbol?': lambda x: isinstance(x, Symbol),
    })
    return env


class Env(dict):
    """Class environment is used to map variable names to their values

    """
    def __init__(self, parms=(), args=(), outer=None):
        self.update(zip(parms, args))
        self.outer = outer
    def find(self, var):
        # Norvig's comment: "find the innermost Env where var appears"
        return self if (var in self) else self.outer.find(var)

global_env = standard_env()

#### REPL: read-expression-print-line

def repl(prompt="cibl> "):
    "Read Expression print line loop"
    while True:
        val = eval(parse(input(prompt)))
        if val is not None:
            print(lispstr(val))
def lispstr(exp):
    "Convert python object into Lisp readable string"
    if isinstance(exp, List):
        return "(" + " ".join(map(lispstr(exp))) + ")"
    else:
        return str(exp)

#### Procedures:

class Procedure(object):
    """User defined Scheme Procedure

    """
    def __init__(self, params, body, env):
        self.params = params
        self.body = body
        self.env = env
    def __call__(self, *args):
        return eval(self.body, Env(self.params, args, self.env))

#### Evaluation

def eval(x, env=global_env):
    """Evaluate an expression in an environment

    """
    if isinstance(x, Symbol): # variable reference
        return env.find(x)[x]
    elif not isinstance(x, List): # constant literal
        return x
    elif x[0] == "quote": # (quote exp)
        (_, exp) = x
        return exp
    elif x[0] == "if": # (if test conseq alt)
        (_, test, conseq, alt) = x
        exp = (conseq if eval(test, env) else alt)
        return eval(exp, env)
    elif x[0] == "define": # (define var exp)
        (_, var, exp) = x
        env[var] = eval(exp, env)
    elif x[0] == "set!":
        (_, var, exp) = x
        env.find(var)[var] = eval(exp, env)
    elif x[0] == "lambda": # (lambda (var ...) body)
        (_, parms, body) = x
        return Procedure(parms, body, env)
    else:
        proc = eval(x[0], env)
        args  = [eval(exp, env) for exp in x[1:]]
        return proc(*args)
