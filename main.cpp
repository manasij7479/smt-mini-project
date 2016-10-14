// #include <iostream>
#include <cvc4/cvc4.h>
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
#include <fstream>
using namespace mm;
int main(int argc, char **argv) {
//   Expr *Post = new BinaryExpr("<", new Var("b"), new IntConst(10), true);
//   Stmt *Prog = new AssignStmt(new Var("b"), 
// 			      new BinaryExpr("+", new Var("a"), new IntConst(1)));
//   auto *Result = Prog->WeakestPrecondition(Post);
//   Result->dump(std::cout);
  
  std::string test = "(1>3)";
  
  if (argc > 1) {
    std::ifstream ifs(argv[1]);
    test.assign( (std::istreambuf_iterator<char>(ifs) ),
                (std::istreambuf_iterator<char>()) );
  }
  
  Stream in(test.c_str(), 0, test.length()-1);
  Result R = ParseProgram(in);
  if (!R.Ptr) {
    std::cout << "FAIL\n";
    return 1;
  }
  
  Program *Prog = R.getAs<Program>();
//   Prog->dump(std::cout);
  Expr *Post = Prog->getPost();
  auto WP = Prog->getStatament()->WeakestPrecondition(Post);
//   WP->dump(std::cout);
  
  CVC4::ExprManager em;
  CVC4::SmtEngine smt(&em);  
  
  CVC4::Expr WeakestPreCond  = WP->Translate(em);
  
  CVC4::Expr GivenPrecondition = Prog->getPre()->Translate(em);
  
  CVC4::Expr Test = em.mkExpr(CVC4::Kind::IMPLIES, WeakestPreCond, GivenPrecondition);
  
  std::cout << "Weakest Precondition : " << WeakestPreCond.toString() << std::endl;
  std::cout << "TEST : " << Test.toString() << std::endl;
  std::cout << "Result : " << smt.query(Test) << std::endl;
}