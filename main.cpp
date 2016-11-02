#include <cvc4/cvc4.h>
#include <iostream>
#include <fstream>
#include "AST.h"
#include "Parser.h"

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
//   Prog->dump(std::cout);
  Expr *Post = Prog->getPost();
  auto WP = Prog->getStatament()->WeakestPrecondition(Post);

  CVC4::ExprManager em;
  CVC4::SmtEngine smt(&em);
  smt.setOption("produce-models", true);
  smt.setOption("output-language", "auto");
  
  std::unordered_map<std::string, CVC4::Expr> SymbolTable;
  std::cout << "Computed WP:\n";
  WP->dump(std::cout);
  std::cout << std::endl;
//   std::cout << WP->Translate(em, SymbolTable).toString();
  WP = WP->Simplify(smt, SymbolTable);
  std::cout << std::endl;
  std::cout << "Simplified WP:\n";
  WP->dump(std::cout);
  std::cout << std::endl;
  
  CVC4::Expr WeakestPreCond  = WP->Translate(em, SymbolTable);
  CVC4::Expr GivenPrecondition = Prog->getPre()->Translate(em, SymbolTable);
  
  CVC4::Expr Test = em.mkExpr(CVC4::Kind::IMPLIES, GivenPrecondition, WeakestPreCond);
   
  std::cout << "Given Program : \n";
  Prog->dump(std::cout);
//   WeakestPreCond = smt.simplify(WeakestPreCond);
//   Test = smt.simplify(Test);
  std::cout << "\nWeakest Precondition : " 
            << WeakestPreCond.toString() << std::endl;
  std::cout << "TEST : " << Test.toString() << std::endl;
  auto Result = smt.query(Test);
  std::cout << "Result : " << Result << std::endl;

  if (!Result.isValid()) {
    std::cout << "Model : \n";
    std::set<std::string> Vars;
    Prog->getPre()->forAllVars([&](std::string VarName){
      Vars.insert(VarName);
    });
    WP->forAllVars([&](std::string VarName){
      Vars.insert(VarName);
    });
    for (auto VarName : Vars) {
      std::cout << VarName << '\t' <<
        smt.getValue(SymbolTable[VarName]).toString() << std::endl;
    }
  }
}
