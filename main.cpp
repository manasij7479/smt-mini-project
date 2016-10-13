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
using namespace mm;
int main() {
  Expr *Post = new BinaryExpr("<", new Var("b"), new IntConst(10), true);
  Stmt *Prog = new AssignStmt(new Var("b"), 
			      new BinaryExpr("+", new Var("a"), new IntConst(1)));
  auto *Result = Prog->WeakestPrecondition(Post);
  Result->dump(std::cout);
}