#ifndef MM_AST_H
#define MM_AST_H
#include <vector>
#include <string>
#include <ostream>
namespace mm {
class Var;
class Expression;
class Expr {
public:
  virtual Expr *Replace(Var *Variable, Expr *Expression) = 0;
  /// Create a new Expression with ``Variable`` replaced with ``Expression``.
  virtual Expr *Copy() = 0;
  virtual void dump(std::ostream &Out) {}
  
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
  Expr *Copy() {
    return new Var(Name);
  }
  void dump(std::ostream &Out) {
    Out << Name;
  }
  std::string getName() {
    return Name;
  }
private:
  std::string Name;
  // int ID; // Will be needed if scopes are introduced.
};
class IntConst : public Expr {
public:
  IntConst(int Value) : Value(Value) {}
  Expr *Replace(Var *Variable, Expr *Expression) {
    return this;
  }
  Expr *Copy() {
    return this;
  }
  void dump(std::ostream &Out) {
    Out << Value;
  }
  int getValue() {
    return Value;
  }
private:
  int Value;
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
private:
  std::string Op;
  Expr *SubExpr;
  bool isBool;
};



class Stmt {
public:
  virtual Expr *WeakestPrecondition(Expr *Post) = 0;
};
class AssignStmt : public  Stmt {
public:
  AssignStmt(Var *LValue, Expr *RValue) : LValue(LValue), RValue(RValue) {}
  Expr *WeakestPrecondition(Expr *Post) {
    return Post->Replace(LValue, RValue);
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
private:
  Expr *Condition;
  Stmt *TrueStmt;
  Stmt *FalseStmt;
};

// class Program {
// public:
// private:
//   Expr *Pre;
//   Stmt *Statement;
//   Expr *Post;
// };

}
#endif