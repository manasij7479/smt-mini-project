#ifndef MM_AST_H
#define MM_AST_H
#include <vector>
#include <string>
#include <ostream>
#include <unordered_map>
#include <cvc4/cvc4.h>
#include <functional>

namespace mm {
class Var;
class Expr;
typedef std::unordered_map<std::string, CVC4::Expr> Table;
class Expr {
public:
  virtual void dump(std::ostream &Out) {}
  virtual CVC4::Expr Translate(CVC4::ExprManager &EM, Table &VARS) = 0;
//   virtual void forAllVars(std::function<void(std::string)> F) = 0;
  virtual Expr *Simplify(CVC4::SmtEngine &SMT, Table &VARS) {
    return this;
  }
  void dbgDmp() {
    dump(std::cerr);
  }
};
class Var : public Expr {
public:
  Var(std::string Name) : Name(Name) {} 

  void dump(std::ostream &Out) {
    Out << Name;
  }
  std::string getName() {
    return Name;
  }
protected:
  std::string Name;
  // int ID; // Will be needed if scopes are introduced.
};
class IntVar : public Var {
public:
  IntVar(std::string Name) : Var(Name) {}
  CVC4::Expr Translate(CVC4::ExprManager &EM, Table &VARS) {
    if (VARS.find(Name) == VARS.end()) {
      VARS[Name] = EM.mkVar(Name, EM.integerType());
    }
    return VARS[Name];
  }
};
class BVVar : public Var {
public:
  BVVar(std::string Name) : Var(Name) {}
  CVC4::Expr Translate(CVC4::ExprManager &EM, Table &VARS) {
    if (VARS.find(Name) == VARS.end()) {
      VARS[Name] = EM.mkVar(Name, EM.mkBitVectorType(32));
    }
    return VARS[Name];
  }
};

class IntConst : public Expr {
public:
  IntConst(int Value) : Value(Value) {}

  void dump(std::ostream &Out) {
    Out << Value;
  }
  int getValue() {
    return Value;
  }
  CVC4::Expr Translate(CVC4::ExprManager &EM, Table &VARS) {
    return EM.mkConst(CVC4::Rational(Value));
  }
private:
  int Value;
};

class BVConst : public Expr {
public:
  BVConst(unsigned int Value) : Value(Value) {}
  void dump(std::ostream &Out) {
    Out << "BV:" << Value;
  }
  unsigned int getValue() {
    return Value;
  }
  CVC4::Expr Translate(CVC4::ExprManager &EM, Table &VARS) {
    return EM.mkConst(CVC4::BitVector(32, Value));
  }
private:
  unsigned int Value;
};

class BoolConst : public Expr {
public:
  BoolConst(bool Value) : Value(Value) {}
  void dump(std::ostream &Out) {
    Out << (Value ? "true" : "false");
  }
  int getValue() {
    return Value;
  }
  CVC4::Expr Translate(CVC4::ExprManager &EM, Table &VARS) {
    return EM.mkConst(Value);
  }
private:
  bool Value;
};

class BinaryExpr : public Expr {
public:
  BinaryExpr(std::string Op, Expr *Left, Expr *Right, bool isBool = false) 
    : Op(Op), Left(Left), Right(Right), isBool(isBool) {}
  void dump(std::ostream &Out) override {
    Out << '(';
    Left->dump(Out);
    Out << Op;
    Right->dump(Out);
    Out << ')';
  }
  CVC4::Expr Translate(CVC4::ExprManager &EM, Table &VARS) override;

  Expr *Simplify(CVC4::SmtEngine &SMT, Table &VARS) override {
    Expr *Left_ = Left->Simplify(SMT, VARS);
    Expr *Right_ = Right->Simplify(SMT, VARS);
    if (Op == "&&") {
      CVC4::Expr L = Left_->Translate(*SMT.getExprManager(), VARS);
      CVC4::Expr R = Right_->Translate(*SMT.getExprManager(), VARS);
      auto Result = SMT.query(L);
      if (Result.isValid()) {
        return Right_;
        
      }
      Result = SMT.query(R);
      if (Result.isValid())
        return Left_;
    }
    else if (Op == "||") {
      CVC4::Expr L = Left_->Translate(*SMT.getExprManager(), VARS);
      CVC4::Expr R = Right_->Translate(*SMT.getExprManager(), VARS);
      auto Result = SMT.query(L);
      if (!Result.isSat())
        return Right_;
      
      Result = SMT.query(R);
      if (!Result.isSat())
        return Left_;
    }
    return new BinaryExpr(Op, Left_, Right_);
  }
private:
  std::string Op;
  Expr *Left;
  Expr *Right;
  bool isBool;
};

class UnaryExpr : public Expr {
public:
  UnaryExpr(std::string Op, Expr *SubExpr, bool isBool = false) 
  : Op(Op), SubExpr(SubExpr), isBool(isBool) {}
  void dump(std::ostream &Out) {
    Out << '(';
    Out << Op;
    SubExpr->dump(Out);
    Out << ')';
  }
  CVC4::Expr Translate(CVC4::ExprManager &EM, Table &VARS);
  Expr *Simplify(CVC4::SmtEngine &SMT, Table &VARS) {
    return new UnaryExpr(Op, SubExpr->Simplify(SMT, VARS));
  }
private:
  std::string Op;
  Expr *SubExpr;
  bool isBool;
};

class UFExpr : public Expr {
public:
  UFExpr(std::string FName, std::vector<Expr *> SubExprs) 
  : FName(FName), SubExprs(SubExprs) {}
  void dump(std::ostream &Out) {
    Out << FName;
    Out << '(';
    for (auto SubExpr : SubExprs) {
      SubExpr->dump(Out);
      Out << ",";
    }
    Out << "\b";
    Out << ')';
  }
  CVC4::Expr Translate(CVC4::ExprManager &EM, Table &VARS) {
    
    if (VARS.find(FName) == VARS.end()) {
//     Expr f_x = em.mkExpr(kind::APPLY_UF, f, x);
      CVC4::Type integer = EM.integerType();
  //     Type boolean = em.booleanType();
      std::vector<CVC4::Type> types(SubExprs.size(), integer);
      CVC4::Type ftype = EM.mkFunctionType(types, integer);
      
      VARS[FName] = EM.mkVar(FName, ftype);
    }
    std::vector<CVC4::Expr> CVC4SubExprs;
    for (Expr *SE : SubExprs) {
      CVC4SubExprs.push_back(SE->Translate(EM, VARS));
    }
    
    return EM.mkExpr(CVC4::Kind::APPLY_UF, VARS[FName] ,CVC4SubExprs);
  }
private:
  std::string FName;
  std::vector<Expr *> SubExprs;
};

namespace {
void tab(std::ostream &out, int level) {
  while (level--)
    out << "  ";
}
}

class Stmt {
public:
  virtual CVC4::Expr WeakestPrecondition(CVC4::Expr Post, CVC4::ExprManager &EM, Table &Vars) = 0;
  virtual void dump(std::ostream &Out, int level = 0) {}
};
class AssignStmt : public  Stmt {
public:
  AssignStmt(Var *LValue, Expr *RValue) : LValue(LValue), RValue(RValue) {}
  CVC4::Expr WeakestPrecondition(CVC4::Expr Post, CVC4::ExprManager &EM, Table &Vars) {
    return Post.substitute(LValue->Translate(EM, Vars), RValue->Translate(EM,Vars));
  }
  void dump(std::ostream &Out, int level) {
    tab(Out, level);
    LValue->dump(Out);
    Out << " = ";
    RValue->dump(Out);
    Out << " ;\n";
  }
private:
  Var *LValue;
  Expr *RValue;
};

class SeqStmt : public Stmt {
public:
  SeqStmt(std::vector<Stmt *> Statements) : Statements(Statements) {};
  CVC4::Expr WeakestPrecondition(CVC4::Expr Post, CVC4::ExprManager &EM, Table &Vars) {
    auto Cur = Post;
    for (auto It = Statements.rbegin(); It != Statements.rend(); ++It) {
      Stmt *Statement = *It;
      Cur = Statement->WeakestPrecondition(Cur, EM, Vars);
    }
    return Cur;
  }
  void dump(std::ostream &Out, int level);
private:
  std::vector<Stmt *> Statements;
};

class CondStmt : public Stmt {
public:
  CondStmt(Expr *Condition,Stmt *TrueStmt, Stmt *FalseStmt)
    : Condition(Condition), TrueStmt(TrueStmt), FalseStmt(FalseStmt) {};
    CVC4::Expr WeakestPrecondition(CVC4::Expr Post, CVC4::ExprManager &EM, Table &Vars) {
      auto C = Condition->Translate(EM, Vars);
      return
      C.impExpr(TrueStmt->WeakestPrecondition(Post, EM, Vars))
       .andExpr(C.notExpr().impExpr(FalseStmt->WeakestPrecondition(Post, EM, Vars)));
  }
  void dump(std::ostream &Out, int level);
private:
  Expr *Condition;
  Stmt *TrueStmt;
  Stmt *FalseStmt;
};

class Program {
public:
  Program (Expr *Pre, Stmt *Statement, Expr *Post)
    : Pre(Pre), Statement(Statement), Post(Post) {}
  Stmt *getStatament() {
    return Statement;
  }
  Expr *getPre() {
    return Pre;
  }
  Expr *getPost() {
    return Post;
  }
  void dump(std::ostream &Out);
private:
  Expr *Pre;
  Stmt *Statement;
  Expr *Post;
};

}
#endif
