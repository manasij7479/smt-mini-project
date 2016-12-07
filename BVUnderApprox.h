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
  auto &em = *SMT.getExprManager();
  CVC4::Result Result;
  auto bv32 = em.mkBitVectorType(32);
  std::set<CVC4::Expr> bvVars;

  for (auto Var : Vars) {
    if (Var.second.getType(false) == bv32) {
      bvVars.insert(Var.second);
    }
  }
  CVC4::Result status;
  size_t CurW = 1;
  while (CurW < 32) {
    CVC4::SmtEngine smt(&em);
    smt.setOption("produce-models", true);
    smt.setOption("produce-assertions", true);
    smt.setOption("incremental", false);
    smt.setOption("produce-unsat-cores", true);
    smt.assertFormula(E.notExpr());
    if (bvVars.size() == 0) {
      status = smt.checkSat();
      break;
    }
    std::set<CVC4::Expr> switches;
    for (auto Var : bvVars) {
      CVC4::Expr bitRange = em.mkConst(CVC4::BitVectorExtract(31, CurW));
      CVC4::Expr extract = em.mkExpr(bitRange, Var);
      CVC4::Expr zerobv = em.mkConst(CVC4::BitVector(32 - CurW, CVC4::Integer(0)));
      CVC4::Expr sw = em.mkExpr(CVC4::Kind::EQUAL, extract, zerobv);
      //std::cout << sw << '\t' << Var.getType() << std::endl;
      switches.insert(sw);
      smt.assertFormula(sw);
    }
    std::cout << std::endl << "Checking assertions: " << std::endl;
    for (auto expr : smt.getAssertions()) {
      std::cout << expr << std::endl;
    }
    std::cout << "Checking satisfiability: ";
    status = smt.checkSat();
    std::cout << status << std::endl;
    if (status == CVC4::Result::Sat::UNSAT) {
      CVC4::UnsatCore uc = smt.getUnsatCore();
      // if (uc.size() == 0) {
      //   break;
      // }
      std::set<CVC4::Expr> newbvVars;
      std::cout << std::endl << "Unsat core: " << std::endl;
      for (auto expr : uc) {
        std::cout << expr << std::endl;
        for (auto sw : switches) {
          if (sw.getId() == expr.getId()) {
            newbvVars.insert(sw.getChildren()[0].getChildren()[0]);
          }
        }
      }
      std::cout << std::endl;
      if (newbvVars.size() == 0) {
        return CVC4::Result::Validity::VALID;
      }
      bvVars = newbvVars;
    } else if (status == CVC4::Result::Sat::SAT) {
        std::cout << "Program is not safe" << std::endl;
        std::cout << "Model : " << std::endl;
        for (auto Var : bvVars) {
          auto VarName = Var.toString();
          std::cout << VarName << '\t' <<
          smt.getValue(Var).toString() << std::endl;
        }
        return CVC4::Result::Validity::INVALID;
    } else {
      return status;
    }
    CurW = isExponential ? CurW << 1 : CurW+1;
  }
  status = SMT.checkSat(E);
  if (status == CVC4::Result::Sat::SAT) {
    std::cout << "Program is not safe" << std::endl;
    std::cout << "Model : " << std::endl;
    for (auto Var : bvVars) {
      auto VarName = Var.toString();
      std::cout << VarName << '\t' <<
      SMT.getValue(Var).toString() << std::endl;
    }
    return CVC4::Result::Validity::INVALID;
  }
  else {
    std::cout << "Program is safe" << std::endl;
    return CVC4::Result::Validity::VALID;
  } 
}
}
#endif
