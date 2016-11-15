#include <cvc4/cvc4.h>
#include <iostream>
#include <fstream>
#include "AST.h"
#include "Parser.h"
#include "ExprSimplifier.h"
#include "BVUnderApprox.h"
using namespace mm;
int main(int argc, char **argv) {
  std::string input;
  std::set<std::string> ExtraArgs;
  if (argc > 1) {
    std::ifstream ifs(argv[1]);
    if (!ifs) {
      std::cerr << "File not found : " << argv[1] << std::endl;
      return 1;
    }
    input.assign( (std::istreambuf_iterator<char>(ifs) ),
                (std::istreambuf_iterator<char>()) );
    for (int i = 2; i < argc; ++i) {
      ExtraArgs.insert(argv[i]);
    }
    if (ExtraArgs.find("simplify") != ExtraArgs.end())
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

  if (SIMP_COND) {
    WeakestPreCond = ApplyAllRecursively(smt, WeakestPreCond);
  }
  CVC4::Expr Test = em.mkExpr(CVC4::Kind::IMPLIES, GivenPrecondition, WeakestPreCond);
   
//   std::cout << "Given Program : \n";
//   Prog->dump(std::cout);

  std::cout << "Weakest Precondition : " 
            << WeakestPreCond.toString() << std::endl;
  std::cout << "TEST : " << Test.toString() << std::endl;
  CVC4::Result Result;
  if (ExtraArgs.find("ua") == ExtraArgs.end()) {
    Result = smt.query(Test);
    if (!Result.isValid() ) {
      std::cout << "Model : \n";
      for (auto Var : SymbolTable) {
        auto VarName = Var.first;
        std::cout << VarName << '\t' <<
        smt.getValue(Var.second).toString() << std::endl;
      }
    }
  } else {
    bool isExponential = ExtraArgs.find("linear") == ExtraArgs.end();
    Result = BVWidthUnderApproxLoop(Test, smt, SymbolTable, isExponential);
  }
  std::cout << "Result : " << Result << std::endl;
}
