#include "ExprMatcher.h"
#include "ExprSimplifier.h"
namespace mm {
  std::pair<CVC4::Expr, bool> PostOrderPatternMatch(CVC4::Expr E, Pattern P, Mutator F) {
    auto EM = E.getExprManager();
    auto Children = E.getChildren();
    bool Changed = false;
    for (auto &C : Children) {
      bool ChangedLocal;
      std::tie(C, ChangedLocal) = PostOrderPatternMatch(C, P, F);
      Changed = Changed || ChangedLocal;
    }
    if (Changed && !Children.empty()) {
      E = EM->mkExpr(E.getKind(), Children);
    }
    
    if (P.Match(E)) {
      return F(E);
    }
    else
      return {E, Changed};
  }
  
  CVC4::Expr PostOrderKindMatch(CVC4::Expr E, CVC4::Kind K, Mutator F) {
    Pattern P(K, {});
    return PostOrderPatternMatch(E, P, F).first;
  }
  
  CVC4::Expr PostOrderTraversal(CVC4::Expr E, Mutator F) {
    Pattern P; // Always passes
    return PostOrderPatternMatch(E, P, F).first;
  }
  
  std::pair<CVC4::Expr, bool> ShortCircuitBinaryAnd(CVC4::SmtEngine &SMT, CVC4::Expr E) {
    if (E.getNumChildren() != 2)
      return {E, false};
    assert(E.getKind() == CVC4::Kind::AND);
    auto L = E.getChildren()[0];
    auto R = E.getChildren()[1];
    auto Result = SMT.query(L);
    if (Result.isValid())
      return {R, true};
    Result = SMT.query(R);
    if (Result.isValid())
      return {L, true};
    return {E, false};
  }
  std::pair<CVC4::Expr, bool> ShortCircuitBinaryOr(CVC4::SmtEngine &SMT, CVC4::Expr E) {
    if (E.getNumChildren() != 2)
      return {E, false};
    assert(E.getKind() == CVC4::Kind::OR);
    auto L = E.getChildren()[0];
    auto R = E.getChildren()[1];
    auto Result = SMT.query(L);
    if (!Result.isSat())
      return {R, true};
    Result = SMT.query(R);
    if (!Result.isSat())
      return {L, true};
    return {E, false};
  }
  
  std::pair<CVC4::Expr, bool> SubsumeBinaryAnd(CVC4::SmtEngine &SMT, CVC4::Expr E) {
    if (E.getNumChildren() != 2)
      return {E, false};
    assert(E.getKind() == CVC4::Kind::AND);
    
    auto L = E.getChildren()[0];
    auto R = E.getChildren()[1];
    
    auto Result = SMT.query(L.impExpr(R));
    if (Result.isValid())
      return {L, true};
    Result = SMT.query(R.impExpr(L));
    if (Result.isValid())
      return {R, true};
    return {E, false};
  }
  
  std::pair<CVC4::Expr, bool> SubsumeBinaryOr(CVC4::SmtEngine &SMT, CVC4::Expr E) {
    if (E.getNumChildren() != 2)
      return {E, false};
    assert(E.getKind() == CVC4::Kind::OR);
    
    auto L = E.getChildren()[0];
    auto R = E.getChildren()[1];
    
    auto Result = SMT.query(L.impExpr(R));
    if (Result.isValid())
      return {R, true};
    Result = SMT.query(R.impExpr(L));
    if (Result.isValid())
      return {L, true};
    return {E, false};
  }
  
  std::pair<CVC4::Expr, bool> SimplifyImplication(CVC4::SmtEngine &SMT, CVC4::Expr E) {
    if (E.getNumChildren() != 2)
      return {E, false};
    assert(E.getKind() == CVC4::Kind::IMPLIES);
    auto L = E.getChildren()[0];
    auto R = E.getChildren()[1];
    
    auto Result = SMT.query(L);
    if (!Result.isSat()) {
      return {SMT.getExprManager()->mkConst(true), true};
    }
    
    Result = SMT.query(R);
    if (Result.isValid())
      return {SMT.getExprManager()->mkConst(true), true};
    
    return {E, false};
  }
  #define F(f) std::bind(f, std::ref(SMT), std::placeholders::_1)
  CVC4::Expr ApplyAllRecursively(CVC4::SmtEngine &SMT, CVC4::Expr E) {
    //Probably too slow to be practical beyond small programs
    E = PostOrderKindMatch(E, CVC4::Kind::AND, F(ShortCircuitBinaryAnd));
    E = PostOrderKindMatch(E, CVC4::Kind::OR, F(ShortCircuitBinaryOr));
    E = PostOrderKindMatch(E, CVC4::Kind::AND, F(SubsumeBinaryAnd));
    E = PostOrderKindMatch(E, CVC4::Kind::OR, F(SubsumeBinaryOr));
    E = PostOrderKindMatch(E, CVC4::Kind::OR, F(SimplifyImplication));
    
    return E;
  }
}
