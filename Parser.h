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
    for (int i = 0; i < Str.length(); ++i) {
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
  return Result(Ptr, in, "Expected Variable");
}
Result ParseIntConst(Stream in) {
  std::string Name = in.loop([](char c){return std::isdigit(c);});
  Expr *Ptr = nullptr; 
  if (Name != "") {
    Ptr = new IntConst(std::stoi(Name));
  }
  return Result(Ptr, in, "Expected Integer Constant");
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
  std::vector<std::string> choices {"+", "-", "*", "/", "&&", "->", "=", "<=", ">=", "<", ">"};
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
  return ret;
  
}
// Result ParseUnaryExpr(Stream in) {
// }
Result ParseExpr(Stream in) {
  Stream Copy = in;
  
  Result var = ParseVar(in);
  if (var.Ptr)
    return var;
  Result ic = ParseIntConst(in);
  if (ic.Ptr)
    return ic;
  Result be = ParseBinaryExpr(in);
    if (be.Ptr)
      return be;
  Result ret(nullptr, Copy, "");
  return ret;
}

// Result ParseStmt(Stream in) { 
// }
// Result ParseAssignStmt(Stream in) {
// }
// Result ParseSeqStmt(Stream in) {
// }
// Result ParseCondStmt(Stream in) {
// }


}
#endif