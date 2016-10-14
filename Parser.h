#ifndef MM_PARSER_H
#define MM_PARSER_H
#include <cctype>
#include <functional>
#include "AST.h"
#include <iostream>
#include <set>
namespace mm {
// Lightweight stream struct, that is basically a char pointer and an index
// Might be deleted if this abstraction doesn't buy us anything
struct Stream {
  Stream(const char* p, int i, int max) : ptr(p), index(i), bounds(max) {}
  const char *ptr;
  int index;
  int bounds;
  char get(bool ignoreWhiteSpace = true) {
    if (index >= bounds)
      return '\0';
    if (std::isspace(ptr[index]) && ignoreWhiteSpace) {
      index++;
      return get(ignoreWhiteSpace);
    }
    return ptr[index++];
  }
  std::string loop(std::function<bool(char)> Pred) {
    skipWhiteSpace();
    std::string Result;
    while (Pred(ptr[index])) {
      Result += ptr[index];
      index++;
    }
    return Result;
  }
  bool fixed(std::string Str) {
    skipWhiteSpace();
    bool failed = false;
    for (unsigned int i = 0; i < Str.length(); ++i) {
      if (Str[i] != ptr[index+i])
        failed = true;
    }
    if (!failed) {
      index += Str.length();
      return true;
    }
    else return false;
  }
  std::string fixed(std::vector<std::string> Choices) {
    for (auto Choice : Choices) {
      if (fixed(Choice)) {
        return Choice;
      }
    }
    return "";
  }
  void skipWhiteSpace() {
    while (index < bounds && std::isspace(ptr[index]))
      index++;
  }
};
struct Result {
  Result(void *Ptr, Stream Str, std::string Err = "") : Ptr(Ptr), Str(Str), Err(Err) {}
  template <typename T>
  T *getAs() {return static_cast<T*>(Ptr);}
  void *Ptr;
  Stream Str;
  std::string Err;
  
};
typedef std::function<Result(Stream)> Parser;

Result ParseExpr(Stream in);

Result ParseVar(Stream in) {
  std::string Name = in.loop([](char c){return std::isalpha(c);});
  Expr *Ptr = nullptr; 
  if (Name != "") {
    Ptr = new Var(Name);
  }
  return Result(Ptr, in, "");
}
Result ParseIntConst(Stream in) {
  std::string Name = in.loop([](char c){return std::isdigit(c);});
  Expr *Ptr = nullptr; 
  if (Name != "") {
    Ptr = new IntConst(std::stoi(Name));
  }
  return Result(Ptr, in, "");
}

Result ParseBoolConst(Stream in) {
  std::vector<std::string> choices {"true", "false"};
  std::string read = in.fixed(choices);
  BoolConst *Ptr = nullptr;
  if (read == "true")
    Ptr = new BoolConst(true);
  else if (read == "false")
    Ptr = new BoolConst(false);
  return Result(Ptr, in, "");
}

Result ParseBinaryExpr(Stream in) {
  Stream Copy = in;
  Result ret(nullptr, Copy, "");
  if (!in.fixed("("))
    return ret;
  Result Left = ParseExpr(in);
  if (!Left.Ptr)
    return ret;
  in = Left.Str;
  std::vector<std::string> choices {"+", "->", "-", "*", "/", "&&", "||", "==", "<=", ">=", "<", ">", "!"};
  std::string Op = in.fixed(choices);
  if (Op == "")
    return ret;
  Result Right = ParseExpr(in);
  if (!Right.Ptr)
    return ret;
  in = Right.Str;
  if (!in.fixed(")"))
    return ret;
  BinaryExpr *result = new BinaryExpr(Op, Left.getAs<Expr>(), Right.getAs<Expr>());
  ret.Ptr = result;
  ret.Str = in;
  return ret;
}

Result ParseUnaryExpr(Stream in) {
  Stream Copy = in;
  Result ret(nullptr, Copy, "");
  if (!in.fixed("("))
    return ret;
  std::vector<std::string> choices {"+", "->", "-", "*", "/", "&&", "||", "==", "<=", ">=", "<", ">", "!"};
  std::string Op = in.fixed(choices);
  if (Op == "")
    return ret;
  Result SubExpr = ParseExpr(in);
  if (!SubExpr.Ptr)
    return ret;
  in = SubExpr.Str;
  if (!in.fixed(")"))
    return ret;
  UnaryExpr *result = new UnaryExpr(Op, SubExpr.getAs<Expr>());
  ret.Ptr = result;
  ret.Str = in;
  return ret;
}

Result ParseExpr(Stream in) {
  Stream Copy = in;
  
  Result be = ParseBinaryExpr(in);
    if (be.Ptr)
      return be;
  Result ue = ParseUnaryExpr(in);
    if (ue.Ptr)
      return ue;
  
  Result bc = ParseBoolConst(in);
  if (bc.Ptr)
    return bc;  
  Result var = ParseVar(in);
  if (var.Ptr)
    return var;
  Result ic = ParseIntConst(in);
  if (ic.Ptr)
    return ic;
  
  Result ret(nullptr, Copy, "");
  return ret;
}

Result ParseStmt(Stream in);

Result ParseAssignStmt(Stream in) {
  Stream Copy = in;
  Result ret(nullptr, Copy, "");
//   if (!in.fixed("#"))
//     return ret;
  Result Left = ParseVar(in);
  if (!Left.Ptr)
    return ret;
  in = Left.Str;
  std::vector<std::string> choices {"="};
  std::string Op = in.fixed(choices);
  if (Op == "")
    return ret;
  Result Right = ParseExpr(in);
  if (!Right.Ptr)
    return ret;
  in = Right.Str;
  if (!in.fixed(";"))
    return ret;
  AssignStmt *result = new AssignStmt(Left.getAs<Var>(), Right.getAs<Expr>());
//   result->dump(std::cout);
  ret.Ptr = result;
  ret.Str = in;
  return ret;
}

Result ParseSeqStmt(Stream in) {
  Stream Copy = in;
  Result ret(nullptr, Copy, "");
  if (!in.fixed("{"))
    return ret;
  std::vector<Stmt *> Internals;
  while (true) {
    if (in.fixed("}"))
      break;
    Result SubStmt = ParseStmt(in);
    if (!SubStmt.Ptr) {
      return ret;
    }
    else {
      in = SubStmt.Str;
      Internals.push_back(SubStmt.getAs<Stmt>());
//       SubStmt.getAs<Stmt>()->dump(std::cout);
    }
  }
//   if (Internals.empty())
//     return ret;
  
  SeqStmt *result = new SeqStmt(Internals);
//   result->dump(std::cout);
  ret.Ptr = result;
  ret.Str = in;
  return ret;
}

Result ParseCondStmt(Stream in) {
  Stream Copy = in;
  Result ret(nullptr, Copy, "");
  if (!in.fixed("if"))
    return ret;
//   if (!in.fixed("("))
//     return ret;
  Result Cond = ParseExpr(in);
  if (!Cond.Ptr)
    return ret;
  in = Cond.Str;
//   Cond.getAs<Expr>()->dump(std::cout);
//   if (!in.fixed(")"))
//     return ret;
  
  if (!in.fixed("{"))
    return ret;
    
  

  Result TrueStmt = ParseStmt(in);
  if (!TrueStmt.Ptr) {
    return ret;
  }
  in = TrueStmt.Str;
//   TrueStmt.getAs<Stmt>()->dump(std::cout);

  if (!in.fixed("}"))
    return ret;
  if (!in.fixed("else"))
    return ret;
  if (!in.fixed("{"))
    return ret;
    
  Result FalseStmt = ParseStmt(in);
  if (!FalseStmt.Ptr)
    return ret;
  in = FalseStmt.Str;
//   FalseStmt.getAs<Stmt>()->dump(std::cout);
  if (!in.fixed("}"))
    return ret;
  CondStmt *result = new CondStmt(Cond.getAs<Expr>(), TrueStmt.getAs<Stmt>(),FalseStmt.getAs<Stmt>());
//   result->dump(std::cout);
  ret.Ptr = result;
  ret.Str = in;
  return ret;
}

Result ParseStmt(Stream in) {
  Stream Copy = in;
  
  Result as = ParseAssignStmt(in);
    if (as.Ptr)
      return as;
  Result ss = ParseSeqStmt(in);
    if (ss.Ptr)
      return ss;
  
  Result cs = ParseCondStmt(in);
  if (cs.Ptr)
    return cs;
  
  Result ret(nullptr, Copy, "");
  return ret;
}

Result ParseProgram(Stream in) {
  Stream Copy = in;
  Result ret(nullptr, Copy, "");
  if (!in.fixed("pre:"))
    return ret;
  
  Result Assume = ParseExpr(in);
  if (!Assume.Ptr)
    return ret;
  in = Assume.Str;
  
  Result Statement = ParseStmt(in);
  if (!Statement.Ptr)
    return ret;
  in = Statement.Str;
  
  if (!in.fixed("post:"))
    return ret;
  
  Result Assert = ParseExpr(in);
  if (!Assert.Ptr)
    return ret;
  in = Assert.Str;
  
  Program *result = new Program(Assume.getAs<Expr>(), Statement.getAs<Stmt>(), Assert.getAs<Expr>());
  
//   result->dump(std::cout);
  ret.Ptr = result;
  ret.Str = in;
  return ret;
}

}
#endif