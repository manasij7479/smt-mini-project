#include "AST.h"
#include "Parser.h"
#include <functional>
#include <vector>
#include <cctype>
#include <iostream>
#include <set>
namespace mm {

typedef std::function<Result(Stream)> Parser;
Parser Choice(std::vector<Parser> Parsers) {
  return [&Parsers](Stream in) -> Result {
    Stream Copy = in;

    for (auto P : Parsers) {
      Result ret = P(in);
      if (ret.Ptr)
        return ret;
    }

    Result ret(nullptr, Copy, "");
    return ret;
  };
}

Result ParseIntVar(Stream in) {
  std::string Name = in.loop([](char c){return std::isalpha(c);});
  Expr *Ptr = nullptr; 
  if (Name != "") {
    Ptr = new IntVar(Name);
  }
  return Result(Ptr, in, "");
}
Result ParseBVVar(Stream in) {
  Stream Copy = in;
  Result ret(nullptr, Copy, "");
  if (!in.fixed("'"))
    return ret;
  std::string Name = in.loop([](char c){return std::isalpha(c);});
  Expr *Ptr = nullptr; 
  if (Name != "") {
    Ptr = new BVVar(Name);
  }
  return Result(Ptr, in, "");
}

Result ParseVar(Stream in) {
  return Choice({ParseIntVar, ParseBVVar})(in);
}

Result ParseIntConst(Stream in) {
  std::string Name = in.loop([](char c){return std::isdigit(c);});
  Expr *Ptr = nullptr; 
  if (Name != "") {
    Ptr = new IntConst(std::stoi(Name));
  }
  return Result(Ptr, in, "");
}

Result ParseBVConst(Stream in) {
  Stream Copy = in;
  Result ret(nullptr, Copy, "");
  if (!in.fixed("'"))
    return ret;
  
  std::string Name = in.loop([](char c){return std::isdigit(c);});
  Expr *Ptr = nullptr; 
  if (Name != "") {
    Ptr = new BVConst(std::stoi(Name));
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
Result ParseExprSansBinExpr(Stream in) {
  return Choice({ParseNestedExpr, ParseUnaryExpr,
    ParseBoolConst, ParseBVConst, ParseVar, ParseIntConst, ParseUFExpr})(in);
}
Result ParseBinaryExpr(Stream in) {
  Stream Copy = in;
  Result ret(nullptr, Copy, "");
  //   if (!in.fixed("("))
  //     return ret;
  Result Left = ParseExprSansBinExpr(in);
  if (!Left.Ptr)
    return ret;
  in = Left.Str;
  std::vector<std::string> choices 
  {"+", "->", "-", "*", "/", 
    "&&", "&",  "||", "|", "==",
    "<=", ">=", "<", ">",
    "'>=", "'>", "'<=", "'<"
  };
  std::string Op = in.fixed(choices);
  if (Op == "")
    return ret;
  Result Right = ParseExpr(in);
  if (!Right.Ptr)
    return ret;
  in = Right.Str;
  //   if (!in.fixed(")"))
  //     return ret;
  BinaryExpr *result = new BinaryExpr(Op, Left.getAs<Expr>(), Right.getAs<Expr>());
  ret.Ptr = result;
  ret.Str = in;
  return ret;
}

Result ParseUnaryExpr(Stream in) {
  Stream Copy = in;
  Result ret(nullptr, Copy, "");
  //   if (!in.fixed("("))
  //     return ret;
  std::vector<std::string> choices {"-", "!", "~"};
  std::string Op = in.fixed(choices);
  if (Op == "")
    return ret;
  Result SubExpr = ParseExpr(in);
  if (!SubExpr.Ptr)
    return ret;
  in = SubExpr.Str;
  //   if (!in.fixed(")"))
  //     return ret;
  UnaryExpr *result = new UnaryExpr(Op, SubExpr.getAs<Expr>());
  ret.Ptr = result;
  ret.Str = in;
  return ret;
}

Result ParseUFExpr(Stream in) {
  Stream Copy = in;
  Result ret(nullptr, Copy, "");
    if (!in.fixed("["))
      return ret;
  Result V = ParseVar(in);
  if (!V.Ptr)
    return ret;
  in = V.Str;
  
  std::vector<Expr *> SubExprs;
  while (true) {
    Result SubExpr = ParseExpr(in);
    if (!SubExpr.Ptr)
      break;
    SubExprs.push_back(SubExpr.getAs<Expr>());
    in = SubExpr.Str;
  }
  
  if (!in.fixed("]"))
    return ret;
  std::string name = V.getAs<Var>()->getName();
  UFExpr *result = new UFExpr(name, SubExprs);
  ret.Ptr = result;
  ret.Str = in;
  return ret;
}

Result ParseNestedExpr(Stream in) {
  Stream Copy  = in;
  Result ret(nullptr, Copy, "");
  if (!in.fixed("("))
    return ret;
  Result SubExpr = ParseExpr(in);
  if (!SubExpr.Ptr)
    return ret;
  in = SubExpr.Str;
  if (!in.fixed(")"))
    return ret;
  SubExpr.Str = in;
  return SubExpr;
}

Result ParseExpr(Stream in) {
  return Choice({ParseBinaryExpr, ParseNestedExpr, ParseUnaryExpr,
                 ParseBoolConst, ParseBVConst, ParseVar, ParseIntConst, ParseUFExpr})(in);
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
  if (!in.fixed("("))
    return ret;
  Result Cond = ParseExpr(in);
  if (!Cond.Ptr)
    return ret;
  in = Cond.Str;
  //   Cond.getAs<Expr>()->dump(std::cout);
  if (!in.fixed(")"))
    return ret;
  
//   if (!in.fixed("{"))
//     return ret;
//     
    
    
  Result TrueStmt = ParseStmt(in);
  if (!TrueStmt.Ptr) {
    return ret;
  }
  in = TrueStmt.Str;
  //   TrueStmt.getAs<Stmt>()->dump(std::cout);
  
//   if (!in.fixed("}"))
//     return ret;
  if (!in.fixed("else")) {
    CondStmt *result = new CondStmt(Cond.getAs<Expr>(), TrueStmt.getAs<Stmt>(), new SeqStmt({}));
  //   result->dump(std::cout);
    ret.Ptr = result;
    ret.Str = in;
    return ret;
  }
//   if (!in.fixed("{"))
//     return ret;
    
  Result FalseStmt = ParseStmt(in);
  if (!FalseStmt.Ptr)
    return ret;
  in = FalseStmt.Str;
  //   FalseStmt.getAs<Stmt>()->dump(std::cout);
//   if (!in.fixed("}"))
//     return ret;
  CondStmt *result = new CondStmt(Cond.getAs<Expr>(), TrueStmt.getAs<Stmt>(),FalseStmt.getAs<Stmt>());
  //   result->dump(std::cout);
  ret.Ptr = result;
  ret.Str = in;
  return ret;
}

Result ParseStmt(Stream in) {
  return Choice({ParseAssignStmt, ParseSeqStmt, ParseCondStmt})(in);
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
