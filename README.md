Specifier -> TYPE

//LVal -> ID
Var -> ID
VarDec -> ID | ID = Exp
VarDecList -> VarDec | VarDec , VarDecList
VarDecStmt -> Specifier VarDecList ;

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

BloclInnerStmtList -> VarDecStmt BlockInnerStmtList | Stmt BlockInnerStmtList | E
BlockStmt -> { VarDecStmtList StmtList }

FuncDef -> Specifier FuncDec BlockStmt

ExtStmt -> VarDecStmt | FuncDecStmt | FuncDef | Lval = Exp
ExtStmtList -> ExtStmt ExtStmtList | E
program -> ExtStmtList
