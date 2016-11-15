#include <cvc4/cvc4.h>
#include <iostream>
#include <fstream>
#include "AST.h"
#include "Parser.h"
//#include "ExprSimplifier.h"
using namespace mm;
int main(int argc, char **argv) {
  std::string input;
  if (argc > 1) {
    std::ifstream ifs(argv[1]);
    if (!ifs) {
      std::cerr << "File not found : " << argv[1] << std::endl;
      return 1;
    }
    input.assign( (std::istreambuf_iterator<char>(ifs) ),
                (std::istreambuf_iterator<char>()) );
    if (argc >= 3 && std::string(argv[2]) == "simplify")
      SIMP_COND = true;
  }
  else {
    std::cerr << "Usage : ./weakest-precond <filename>" << std::endl;
    return 1;
  }
  
  Stream in(input.c_str(), 0, input.length()-1);
  Result R = ParseProgram(in);
  if (!R.Ptr) {
    std::cout << "FAIL\n";
    return 1;
  }
  
  Program *Prog = R.getAs<Program>();

  CVC4::ExprManager em;
  CVC4::SmtEngine smt(&em);
  smt.setOption("produce-models", true);
  smt.setOption("output-language", "auto");
  
  std::unordered_map<std::string, CVC4::Expr> SymbolTable;
  
  CVC4::Expr Post = Prog->getPost()->Translate(em, SymbolTable);
  CVC4::Expr WeakestPreCond  = Prog->getStatament()->WeakestPrecondition(Post, smt, SymbolTable);
  
  CVC4::Expr GivenPrecondition = Prog->getPre()->Translate(em, SymbolTable);

  /*
  if (SIMP_COND) {
    WeakestPreCond = ApplyAllRecursively(smt, WeakestPreCond);
  }
  */
  CVC4::Expr Test = em.mkExpr(CVC4::Kind::IMPLIES, GivenPrecondition, WeakestPreCond);
   
  std::cout << "Given Program : \n";
  Prog->dump(std::cout);

  std::cout << "\nWeakest Precondition : " 
            << WeakestPreCond.toString() << std::endl;
  std::cout << "TEST : " << Test.toString() << std::endl;
  auto Result = smt.query(Test);
  std::cout << "Result : " << Result << std::endl;

  if (!Result.isValid()) {
    std::cout << "Model : \n";
    for (auto Var : SymbolTable) {
      auto VarName = Var.first;
      std::cout << VarName << '\t' <<
        smt.getValue(Var.second).toString() << std::endl;
    }
  }

  CVC4::Expr Pre = Prog->getPre()->Translate(em, SymbolTable);
  CVC4::Expr StrongestPostCond  = Prog->getStatament()->StrongestPostcondition(Pre, smt, SymbolTable);
  
  CVC4::Expr GivenPostcondition = Prog->getPost()->Translate(em, SymbolTable);

  /*
  if (SIMP_COND) {
    WeakestPreCond = ApplyAllRecursively(smt, WeakestPreCond);
  }
  */
  Test = em.mkExpr(CVC4::Kind::IMPLIES, StrongestPostCond, GivenPostcondition);
   
  std::cout << "Given Program : \n";
  Prog->dump(std::cout);

  std::cout << "\nStrongest Postcondition : " 
            << StrongestPostCond.toString() << std::endl;
  std::cout << "TEST : " << Test.toString() << std::endl;
  Result = smt.query(Test);
  std::cout << "Result : " << Result << std::endl;

  if (!Result.isValid()) {
    std::cout << "Model : \n";
    for (auto Var : SymbolTable) {
      auto VarName = Var.first;
      std::cout << VarName << '\t' <<
        smt.getValue(Var.second).toString() << std::endl;
    }
  }
}
