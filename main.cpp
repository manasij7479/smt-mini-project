#include <cvc4/cvc4.h>
#include <iostream>
#include <fstream>
#include "AST.h"
#include "Parser.h"

using namespace mm;
int main(int argc, char **argv) {
  std::string test;
  if (argc > 1) {
    std::ifstream ifs(argv[1]);
    test.assign( (std::istreambuf_iterator<char>(ifs) ),
                (std::istreambuf_iterator<char>()) );
  }
  else {
    std::cerr << "Usage : ./weakest-precond <filename>" << std::endl;
    return 1;
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

  
  CVC4::ExprManager em;
  CVC4::SmtEngine smt(&em);  
  
  std::unordered_map<std::string, CVC4::Expr> SymbolTable;
  
  CVC4::Expr WeakestPreCond  = WP->Translate(em, SymbolTable);
  
  CVC4::Expr GivenPrecondition = Prog->getPre()->Translate(em, SymbolTable);
  
  CVC4::Expr Test = em.mkExpr(CVC4::Kind::IMPLIES, GivenPrecondition, WeakestPreCond);
   
  std::cout << "Weakest Precondition : " << WeakestPreCond.toString() << std::endl;
  std::cout << "TEST : " << Test.toString() << std::endl;
  std::cout << "Result : " << smt.query(Test) << std::endl;
}