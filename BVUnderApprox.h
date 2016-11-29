#ifndef BV_UA
#define BV_UA
#include <cvc4/cvc4.h>
#include <unordered_map>
#include "ExprMatcher.h"
namespace mm {
#define Curry(f, a, b) std::bind(f, std::ref(a), std::ref(b) ,std::placeholders::_1)  
  
std::pair<CVC4::Expr, bool> TruncateBVConst(CVC4::SmtEngine &SMT, size_t Width,  CVC4::Expr E) {
  if (E.getKind() == CVC4::Kind::CONST_BITVECTOR) {
    auto &EM = *SMT.getExprManager();
    auto bvTWidth = EM.mkBitVectorType(Width);
    unsigned int value = E.getConst<CVC4::BitVector>().getValue().getUnsignedInt();
    return {EM.mkConst(CVC4::BitVector(Width, value)), true};
  } else {
    return {E, true};
  }
}

CVC4::Result BVWidthUnderApproxLoop(CVC4::Expr E, CVC4::SmtEngine &SMT, std::unordered_map<std::string,
                                    CVC4::Expr> Vars, bool isExponential = true) {
  size_t CurW = 1;
  auto &EM = *SMT.getExprManager();
  CVC4::Result Result;
  auto bv32 = EM.mkBitVectorType(32);
  while (CurW <= 32) {
    auto TempExpr = E;
    std::set<CVC4::Expr> NewVars;
    for (auto Pair : Vars) {
      if (Pair.second.getType(false) == bv32) {
        auto NewVar = EM.mkVar(Pair.first + "_" + std::to_string(CurW),
                 EM.mkBitVectorType(CurW));
        
        TempExpr = TempExpr.substitute(
          Pair.second, NewVar);
        TempExpr = PostOrderTraversal(TempExpr, Curry(TruncateBVConst, SMT, CurW));
        NewVars.insert(NewVar);
      }
    }
    std::cout << "UA TEST : " << TempExpr.toString() << std::endl;
    Result = SMT.query(TempExpr);
    if (!Result.isValid()) {
      std::cout << "UA Success : " << TempExpr.toString() << std::endl;
      
      std::cout << "Model : \n";
      for (auto Var : NewVars) {
        auto VarName = Var.toString();
        std::cout << VarName << '\t' <<
        SMT.getValue(Var).toString() << std::endl;
      }
      
      return Result;
    }
    if (isExponential) {
      CurW <<= 1;
    }
    else {
      CurW++;
    }
  }
  std::cout << "UA Fail" << std::endl;
  return Result;
}
}
#endif
