// #include <iostream>
// #include <cvc4/cvc4.h>
// using namespace CVC4;
// int main() {
//   ExprManager em;
//   Expr helloworld = em.mkVar("Hello World!", em.booleanType());
//   SmtEngine smt(&em);
//   std::cout << helloworld << " is " << smt.query(helloworld) << std::endl;
//   return 0;
// }
#include <iostream>
#include "AST.h"
#include "Parser.h"
using namespace mm;
int main() {
//   Expr *Post = new BinaryExpr("<", new Var("b"), new IntConst(10), true);
//   Stmt *Prog = new AssignStmt(new Var("b"), 
// 			      new BinaryExpr("+", new Var("a"), new IntConst(1)));
//   auto *Result = Prog->WeakestPrecondition(Post);
//   Result->dump(std::cout);
  
  std::string test = "(x^1)";
  Stream in(test.c_str(), 0, test.length()-1);
  Result R = ParseBinaryExpr(in);
  if (R.Ptr) {
    R.getAs<BinaryExpr>()->dump(std::cout);
  }
}