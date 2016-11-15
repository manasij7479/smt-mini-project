#ifndef MM_SIMP_H
#define MM_SIMP_H
#include <cvc4/cvc4.h>
#include "ExprMatcher.h"
namespace mm {

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

#define F(f) std::bind(f, std::ref(SMT), std::placeholders::_1)

CVC4::Expr ApplyAllRecursively(CVC4::SmtEngine &SMT, CVC4::Expr E) {
  //Probably too slow to be practical beyond small programs
  E = PostOrderKindMatch(E, CVC4::Kind::AND, F(ShortCircuitBinaryAnd));
  E = PostOrderKindMatch(E, CVC4::Kind::OR, F(ShortCircuitBinaryOr));
  E = PostOrderKindMatch(E, CVC4::Kind::AND, F(SubsumeBinaryAnd));
  E = PostOrderKindMatch(E, CVC4::Kind::OR, F(SubsumeBinaryOr));
  
  return E;
}
 
}
#endif
