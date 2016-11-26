#ifndef MM_AST_H
#define MM_AST_H
#include <vector>
#include <string>
#include <ostream>
#include <unordered_map>
#include <cvc4/cvc4.h>
#include <functional>
#include "Summary.h"

extern bool SIMP_COND; // TODO : Remove global
namespace mm {
class DefStmt;
}
std::unordered_map<std::string, mm::DefStmt *>& FDEFS();
std::unordered_map<std::string, mm::Summary>& SUMMARIES();
namespace mm {
class Var;
class Expr;
class Expr {
public:
  virtual void dump(std::ostream &Out) {}
  virtual CVC4::Expr Translate(CVC4::ExprManager &EM, Table &VARS) = 0;
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
  virtual CVC4::Expr WeakestPrecondition(CVC4::Expr Post, CVC4::SmtEngine &SMT, Table &Vars) = 0;
  virtual CVC4::Expr StrongestPostcondition(CVC4::Expr Pre, CVC4::SmtEngine &SMT, Table &Vars) = 0;
  virtual void dump(std::ostream &Out, int level = 0) {}
  virtual Summary PropagateWPSummary(Summary S, CVC4::SmtEngine &SMT, Table &Vars) = 0;
  virtual void AndAllAsserts(CVC4::Expr &Propagate,  CVC4::SmtEngine &SMT, Table &Vars) {}
};
class AssignStmt : public  Stmt {
public:
  AssignStmt(Var *LValue, Expr *RValue) : LValue(LValue), RValue(RValue) {}
  CVC4::Expr WeakestPrecondition(CVC4::Expr Post, CVC4::SmtEngine &SMT, Table &Vars) {
    auto &EM = *SMT.getExprManager();
    return Post.substitute(LValue->Translate(EM, Vars), RValue->Translate(EM,Vars));
  }
  CVC4::Expr StrongestPostcondition(CVC4::Expr Pre, CVC4::SmtEngine &SMT, Table &Vars) {
    auto &EM = *SMT.getExprManager();
    std::string Name = LValue->getName();
    CVC4::Type T = EM.integerType();
//     if (!Vars[Name].isNull()) {
//       T = Vars[Name].getType(false);
//     }
    CVC4::Expr boundVar = EM.mkBoundVar("boundVar_"+Name, T);
    std::vector<CVC4::Expr> boundVars;
    boundVars.push_back(boundVar);
    CVC4::Expr boundVarList = EM.mkExpr(CVC4::kind::BOUND_VAR_LIST, boundVar);
    CVC4::Expr predicate1 = EM.mkExpr(CVC4::Kind::EQUAL, 
                                      LValue->Translate(EM, Vars),
                                      RValue->Translate(EM, Vars).substitute(LValue->Translate(EM, Vars), boundVar));
    CVC4::Expr predicate2 = Pre.substitute(LValue->Translate(EM, Vars), boundVar);
    CVC4::Expr predicate = EM.mkExpr(CVC4::Kind::AND, predicate1, predicate2);
    CVC4::Expr formula = EM.mkExpr(CVC4::Kind::EXISTS, boundVarList, predicate);
//     return SMT.doQuantifierElimination(formula, false);
    return formula;
  }
  void dump(std::ostream &Out, int level) {
    tab(Out, level);
    LValue->dump(Out);
    Out << " = ";
    RValue->dump(Out);
    Out << " ;\n";
  }
  Summary PropagateWPSummary(Summary S, CVC4::SmtEngine &SMT, Table &Vars) {
    S.substitute(LValue->getName(), RValue->Translate(*SMT.getExprManager(), Vars), Vars);
    return S;
  }
private:
  Var *LValue;
  Expr *RValue;
};

class SeqStmt : public Stmt {
public:
  SeqStmt(std::vector<Stmt *> Statements) : Statements(Statements) {};
  CVC4::Expr WeakestPrecondition(CVC4::Expr Post, CVC4::SmtEngine &SMT, Table &Vars) {
    auto Cur = Post;
    for (auto It = Statements.rbegin(); It != Statements.rend(); ++It) {
      Stmt *Statement = *It;
      Cur = Statement->WeakestPrecondition(Cur, SMT, Vars);
    }
    
    return Cur;
  }
  CVC4::Expr StrongestPostcondition(CVC4::Expr Pre, CVC4::SmtEngine &SMT, Table &Vars) {
    auto Cur = Pre;
    for (auto It = Statements.begin(); It != Statements.end(); ++It) {
      Stmt *Statement = *It;
      Cur = Statement->StrongestPostcondition(Cur, SMT, Vars);
    }
    return Cur;
  }
  void dump(std::ostream &Out, int level);
  Summary PropagateWPSummary(Summary S, CVC4::SmtEngine &SMT, Table &Vars) {
    for (auto It = Statements.rbegin(); It != Statements.rend(); ++It) {
      Stmt *Statement = *It;
      S = Statement->PropagateWPSummary(S, SMT, Vars);
    }
    return S;
  }
  void AndAllAsserts(CVC4::Expr &Propagate,  CVC4::SmtEngine &SMT, Table &Vars) {
    for (auto *Child : Statements) {
      Child->AndAllAsserts(Propagate, SMT, Vars);
    }
  }
private:
  std::vector<Stmt *> Statements;
};
class CondStmt : public Stmt {
public:
  CondStmt(Expr *Condition,Stmt *TrueStmt, Stmt *FalseStmt)
    : Condition(Condition), TrueStmt(TrueStmt), FalseStmt(FalseStmt) {};
    CVC4::Expr WeakestPrecondition(CVC4::Expr Post, CVC4::SmtEngine &SMT, Table &Vars) {
      auto &EM = *SMT.getExprManager();
      auto C = Condition->Translate(EM, Vars);

      auto TWP = [&]() {
        static auto Result = TrueStmt->WeakestPrecondition(Post, SMT, Vars);
        return Result;
      };
      auto FWP = [&]() {
        static auto Result = FalseStmt->WeakestPrecondition(Post, SMT, Vars);
        return Result;
      };
      // Above trick is for lazy evaluation.
      // Works for most compilers in production but not guaranteed by the standard
      // for lambda functions (IIRC).
      
      auto TAsserts = Post;
      TrueStmt->AndAllAsserts(TAsserts, SMT, Vars);
      auto FAsserts = Post;
      FalseStmt->AndAllAsserts(FAsserts, SMT, Vars);
      auto TValid = SMT.query(TAsserts).isValid();
      auto FValid = SMT.query(FAsserts).isValid();
      if (TValid) {
        if (FValid) {
          return SMT.getExprManager()->mkConst(true);
        }
        return FWP();
      } else if (FValid) {
        return TWP();
      }
      
      if (SIMP_COND) {
        if (SMT.query(C).isValid())
          return TWP();
        if (SMT.query(C.notExpr()).isValid())
          return FWP();
        if (SMT.query(TWP().iffExpr(FWP())).isValid())
          return TWP();
      }
      
      return C.impExpr(TWP()).andExpr(C.notExpr().impExpr(FWP()));
    }

    CVC4::Expr StrongestPostcondition(CVC4::Expr Pre, CVC4::SmtEngine &SMT, Table &Vars) {
      auto &EM = *SMT.getExprManager();
      auto C = Condition->Translate(EM, Vars);
      auto TSP = TrueStmt->StrongestPostcondition(Pre.andExpr(C), SMT, Vars);
      auto FSP = FalseStmt->StrongestPostcondition(Pre.andExpr(C.notExpr()), SMT, Vars);
      if (SIMP_COND) {
        auto R = SMT.query(TSP.iffExpr(FSP));
        if (R.isValid())
          return TSP;
      }
      return TSP.orExpr(FSP);
    }
  
  void dump(std::ostream &Out, int level);
  Summary PropagateWPSummary(Summary S, CVC4::SmtEngine &SMT, Table &Vars) {
    auto &EM = *SMT.getExprManager();
    Summary T, F;
    T = TrueStmt->PropagateWPSummary(T, SMT, Vars);
    F = FalseStmt->PropagateWPSummary(F, SMT, Vars);
    S.branch(Condition->Translate(EM, Vars), T, F, SMT, Vars);
    return S;
  }
  void AndAllAsserts(CVC4::Expr &Propagate,  CVC4::SmtEngine &SMT, Table &Vars) {
    TrueStmt->AndAllAsserts(Propagate, SMT, Vars);
    FalseStmt->AndAllAsserts(Propagate, SMT, Vars);
  }
private:
  Expr *Condition;
  Stmt *TrueStmt;
  Stmt *FalseStmt;
};

class AssertStmt : public Stmt {
public:
  AssertStmt(Expr *Assertion) : Assertion(Assertion) {}
  CVC4::Expr WeakestPrecondition(CVC4::Expr Post, CVC4::SmtEngine &SMT, Table &Vars) {
    return Post.andExpr(Assertion->Translate(*SMT.getExprManager(), Vars));
  }
  
  CVC4::Expr StrongestPostcondition(CVC4::Expr Pre, CVC4::SmtEngine &SMT, Table &Vars) {
    return Pre.andExpr(Assertion->Translate(*SMT.getExprManager(), Vars));
  }
  
  void dump(std::ostream &Out, int level) {
    tab(Out, level);
    Assertion->dump(Out);
    Out << "\n";
  }
  Summary PropagateWPSummary(Summary S, CVC4::SmtEngine &SMT, Table &Vars) {
    auto E = /*S.apply(*/Assertion->Translate(*SMT.getExprManager(), Vars)/*, Vars)*/;
    S.addPredicate(E, SMT, Vars);
    return S;
  }
  virtual void AndAllAsserts(CVC4::Expr &Propagate,  CVC4::SmtEngine &SMT, Table &Vars) {
    Propagate = Propagate.andExpr(Assertion->Translate(*SMT.getExprManager(), Vars));
  }
private:
  Expr *Assertion;
};

class DefStmt : public Stmt {
public:
  DefStmt(std::string Name, Stmt *Body) : Name(Name), Body(Body) {}
  
  CVC4::Expr WeakestPrecondition(CVC4::Expr Post, CVC4::SmtEngine &SMT, Table &Vars) {
    return Post;
  }
  
  CVC4::Expr StrongestPostcondition(CVC4::Expr Pre, CVC4::SmtEngine &SMT, Table &Vars) {
    return Pre;
  }
  void dump(std::ostream &Out, int level) {
    tab(Out, level);
    Out << "def: " << Name << "\n";
    Body->dump(Out, level);
//     Out << "\n";
  }
  Summary PropagateWPSummary(Summary S, CVC4::SmtEngine &SMT, Table &Vars) {
    return S;
  }
  std::string getName() {
    return Name;
  }
  Summary ComputeSummary(CVC4::SmtEngine &SMT, Table &Vars) {
    if (SUMMARIES().find(Name) == SUMMARIES().end()) {
      Summary S;
      SUMMARIES()[Name] = Body->PropagateWPSummary(S, SMT, Vars);
      std::cout << "Summary for " << Name << std::endl;
      SUMMARIES()[Name].dump(std::cout, SMT);
    }
    return SUMMARIES()[Name];
  }
  Stmt *getBody() {
    return Body;
  }
private:
  std::string Name;
  Stmt *Body;
};

class CallStmt : public Stmt {
public:
  CallStmt(DefStmt *Func) : Func(Func) {}
  CVC4::Expr WeakestPrecondition(CVC4::Expr Post, CVC4::SmtEngine &SMT, Table &Vars) {
    auto S = Func->ComputeSummary(SMT, Vars);
    auto result = S.apply(Post, Vars);
    auto Pred = S.getPredicate(SMT);
    auto E = result.andExpr(Pred);
    return E;
  }
  
  CVC4::Expr StrongestPostcondition(CVC4::Expr Pre, CVC4::SmtEngine &SMT, Table &Vars) {
    return Pre; //TODO
  }
  void dump(std::ostream &Out, int level) {
    tab(Out, level);
    Out << "call: " << Func->getName() << "\n";
  }
  Summary PropagateWPSummary(Summary S, CVC4::SmtEngine &SMT, Table &Vars) {
    return Func->getBody()->PropagateWPSummary(S, SMT, Vars);
  }
  void AndAllAsserts(CVC4::Expr &Propagate,  CVC4::SmtEngine &SMT, Table &Vars) {
    Func->getBody()->AndAllAsserts(Propagate, SMT, Vars);
  }
private:
  DefStmt *Func;
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
