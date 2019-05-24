Specifier -> TYPE

LVal -> ID
VarDec -> ID | ID = Exp
VarDecList -> VarDec | VarDec , VarDecList
VarDecStmt -> Specifier VarDecList ;
VarDecStmtList -> VarDecStmt VarDecStmtList | E

ParamDec -> Specifier ID
ParmaList -> ParamDec | ParamDec , ParamList
FuncDec -> ID () | ID (ParamList)
FuncDecStmt -> Specifier FuncDec ;

Exp -> LVal = Exp
    | Exp && Exp
    | Exp || Exp
    | Exp < Exp
    | Exp > Exp
    | Exp == Exp
    | Exp != Exp
    | Exp <= Exp
    | Exp >= Exp
    | Exp + Exp
    | Exp - Exp
    | Exp * Exp
    | Exp / Exp
    | (Exp)
    | -Exp
    | !Exp
    | ID
    | ID ()
    | ID (ArgList)
    | INT
    | FLOAT
    | CHAR

ArgList -> Exp | Exp , ArgList

Stmt -> Exp ;
    | BlockStmt
    | return Exp ;
    | if (Exp) Stmt
    | if (Exp) Stmt else Stmt
    | while (Exp) Stmt

StmtList -> Stmt StmtList | E

BlockStmt -> { VarDecStmtList StmtList }

FuncDef -> Specifier FuncDec BlockStmt

BareStmt -> VarDecStmt | FuncDecStmt | FuncDef | Lval = Exp
BareStmtList -> BareStmt BareStmtList | E
program -> BareStmtList
