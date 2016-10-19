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

class Expr {
public:
  virtual Expr *Replace(Var *Variable, Expr *Expression) = 0;
  /// Create a new Expression with ``Variable`` replaced with ``Expression``.
  virtual Expr *Copy() = 0;
  virtual void dump(std::ostream &Out) {}
  virtual CVC4::Expr Translate(CVC4::ExprManager &EM, std::unordered_map<std::string, CVC4::Expr> &VARS) = 0;
  virtual void forAllVars(std::function<void(std::string)> F) {}
};
class Var : public Expr {
public:
  Var(std::string Name) : Name(Name) {} 
  Expr *Replace(Var *Variable, Expr *Expression) {
    if (Variable->Name == Name) {
      return Expression->Copy();
    } else {
      return this->Copy();
    }
  }

  void dump(std::ostream &Out) {
    Out << Name;
  }
  std::string getName() {
    return Name;
  }
  
  void forAllVars(std::function<void(std::string)> F) {
    F(Name);
  }
protected:
  std::string Name;
  // int ID; // Will be needed if scopes are introduced.
};
class IntVar : public Var {
public:
  IntVar(std::string Name) : Var(Name) {}
  Expr *Copy() {
    return new IntVar(Name);
  }
  CVC4::Expr Translate(CVC4::ExprManager &EM, std::unordered_map<std::string, CVC4::Expr> &VARS) {
    if (VARS.find(Name) == VARS.end()) {
      VARS[Name] = EM.mkVar(Name, EM.integerType());
    }
    return VARS[Name];
  }
};
class BVVar : public Var {
public:
  BVVar(std::string Name) : Var(Name) {}
  Expr *Copy() {
    return new BVVar(Name);
  }
  CVC4::Expr Translate(CVC4::ExprManager &EM, std::unordered_map<std::string, CVC4::Expr> &VARS) {
    if (VARS.find(Name) == VARS.end()) {
      VARS[Name] = EM.mkVar(Name, EM.mkBitVectorType(32));
    }
    return VARS[Name];
  }
};

class Const : public Expr {
public:
  Expr *Replace(Var *Variable, Expr *Expression) {
    return this;
  }
  Expr *Copy() {
    return this;
  }  
};

class IntConst : public Const {
public:
  IntConst(int Value) : Value(Value) {}

  void dump(std::ostream &Out) {
    Out << Value;
  }
  int getValue() {
    return Value;
  }
  CVC4::Expr Translate(CVC4::ExprManager &EM, std::unordered_map<std::string, CVC4::Expr> &VARS) {
    return EM.mkConst(CVC4::Rational(Value));
  }
private:
  int Value;
};

class BVConst : public Const {
public:
  BVConst(unsigned int Value) : Value(Value) {}
  void dump(std::ostream &Out) {
    Out << "BV:" << Value;
  }
  unsigned int getValue() {
    return Value;
  }
  CVC4::Expr Translate(CVC4::ExprManager &EM, std::unordered_map<std::string, CVC4::Expr> &VARS) {
    return EM.mkConst(CVC4::BitVector(32, Value));
  }
private:
  unsigned int Value;
};

class BoolConst : public Const {
public:
  BoolConst(bool Value) : Value(Value) {}
  void dump(std::ostream &Out) {
    Out << (Value ? "true" : "false");
  }
  int getValue() {
    return Value;
  }
  CVC4::Expr Translate(CVC4::ExprManager &EM, std::unordered_map<std::string, CVC4::Expr> &VARS) {
    return EM.mkConst(Value);
  }
private:
  bool Value;
};

class BinaryExpr : public Expr {
public:
  BinaryExpr(std::string Op, Expr *Left, Expr *Right, bool isBool = false) 
    : Op(Op), Left(Left), Right(Right), isBool(isBool) {}
  Expr *Replace(Var *Variable, Expr *Expression) {
    return new BinaryExpr(Op, Left->Replace(Variable, Expression),
        Right->Replace(Variable, Expression), isBool);
  }
  Expr *Copy() {
    return new BinaryExpr(Op, Left, Right, isBool);
  }
  void dump(std::ostream &Out) {
    Out << '(';
    Left->dump(Out);
    Out << Op;
    Right->dump(Out);
    Out << ')';
  }
  CVC4::Expr Translate(CVC4::ExprManager &EM, std::unordered_map<std::string, CVC4::Expr> &VARS) {
    std::unordered_map<std::string, CVC4::Kind> Map = {
      {"+", CVC4::Kind::PLUS},
      {"-", CVC4::Kind::MINUS},
      {"->", CVC4::Kind::IMPLIES},
      {"*", CVC4::Kind::MULT},
      {"/", CVC4::Kind::DIVISION},
      {"&&", CVC4::Kind::AND},
      {"||", CVC4::Kind::OR},
      {"==", CVC4::Kind::EQUAL},
      {"<=", CVC4::Kind::LEQ},
      {">=", CVC4::Kind::GEQ},
      {"<", CVC4::Kind::LT},
      {">", CVC4::Kind::GT},
      {"!", CVC4::Kind::NOT},
      {"&", CVC4::Kind::BITVECTOR_AND},
      {"|", CVC4::Kind::BITVECTOR_OR},
      {"'>", CVC4::Kind::BITVECTOR_UGT},
      {"'>=", CVC4::Kind::BITVECTOR_UGE},
      {"'<", CVC4::Kind::BITVECTOR_ULT},
      {"'<=", CVC4::Kind::BITVECTOR_ULE},
      
    };
    return EM.mkExpr(Map[Op], Left->Translate(EM, VARS), Right->Translate(EM, VARS));
  }
  void forAllVars(std::function<void(std::string)> F) {
    Left->forAllVars(F);
    Right->forAllVars(F);
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
  Expr *Replace(Var *Variable, Expr *Expression) {
    return new UnaryExpr(Op, SubExpr->Replace(Variable, Expression), isBool);
  }
  Expr *Copy() {
    return new UnaryExpr(Op, SubExpr, isBool);
  }
  void dump(std::ostream &Out) {
    Out << '(';
    Out << Op;
    SubExpr->dump(Out);
    Out << ')';
  }
  CVC4::Expr Translate(CVC4::ExprManager &EM, std::unordered_map<std::string, CVC4::Expr> &VARS) {
//     {"+", "->", "-", "*", "/", "&&", "||", "==", "<=", ">=", "<", ">", "!"}
    std::unordered_map<std::string, CVC4::Kind> Map = {
      {"-", CVC4::Kind::UMINUS},
      {"!", CVC4::Kind::NOT},
      {"~", CVC4::Kind::BITVECTOR_NOT}
    };
    return EM.mkExpr(Map[Op], SubExpr->Translate(EM, VARS));
  }
  void forAllVars(std::function<void(std::string)> F) {
    SubExpr->forAllVars(F);
  }
private:
  std::string Op;
  Expr *SubExpr;
  bool isBool;
};

void tab(std::ostream &out, int level) {
  while (level--)
    out << "  ";
}

class Stmt {
public:
  virtual Expr *WeakestPrecondition(Expr *Post) = 0;
  virtual void dump(std::ostream &Out, int level = 0) {}
};
class AssignStmt : public  Stmt {
public:
  AssignStmt(Var *LValue, Expr *RValue) : LValue(LValue), RValue(RValue) {}
  Expr *WeakestPrecondition(Expr *Post) {
    return Post->Replace(LValue, RValue);
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
  Expr *WeakestPrecondition(Expr *Post) {
    auto *Cur = Post;
    for (auto It = Statements.rbegin(); It != Statements.rend(); ++It) {
      Stmt *Statement = *It;
      Cur = Statement->WeakestPrecondition(Cur);
    }
    return Cur;
  }
  void dump(std::ostream &Out, int level) {
    tab(Out, level);
    Out << "{\n";
    for (auto *Simple : Statements) {
      Simple->dump(Out, level+1);
    }
    tab(Out, level);
    Out << "}\n";
  }
private:
  std::vector<Stmt *> Statements;
};

class CondStmt : public Stmt {
public:
  CondStmt(Expr *Condition,Stmt *TrueStmt, Stmt *FalseStmt)
    : Condition(Condition), TrueStmt(TrueStmt), FalseStmt(FalseStmt) {};
  Expr *WeakestPrecondition(Expr *Post) {
    return new BinaryExpr("&&", 
      new BinaryExpr("->", Condition, TrueStmt->WeakestPrecondition(Post)),
      new BinaryExpr("->", new UnaryExpr("!", Condition), 
      FalseStmt->WeakestPrecondition(Post)), true);
  }
  void dump(std::ostream &Out, int level) {
    tab(Out, level);
    Out << "if ( ";
    Condition->dump(Out);
    Out << " ) { \n";
    TrueStmt->dump(Out, level+1);
    tab(Out, level);
    Out << "} else {\n";
    FalseStmt->dump(Out, level+1);
    tab(Out, level);
    Out << "}\n";
  }
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
  void dump(std::ostream &Out) {
    Out << "pre:\t";
    Pre->dump(Out);
    Out << "\nstmt:\n";
    Statement->dump(Out, 0);
    Out << "post:\t";
    Post->dump(Out);
    Out << std::endl;
  }
private:
  Expr *Pre;
  Stmt *Statement;
  Expr *Post;
};

}
#endif
