#ifndef MM_MATCHER_H
#define MM_MATCHER_H
#include <cvc4/cvc4.h>
#include <functional>
#include <map>
namespace mm {
typedef std::function<std::pair<CVC4::Expr, bool>(CVC4::Expr)> Mutator;

class Pattern {
public:
  Pattern() {}
  Pattern(CVC4::Type T, std::map<int, Pattern> Children)
    : T(T), Children(Children), isTypePattern(true) {}
  Pattern(CVC4::Kind K, std::map<int, Pattern> Children)
    : K(K), Children(Children), isTypePattern(false) {}
  
  bool Match(CVC4::Expr E) {
    if (isTypePattern) {
      if (E.getType() != T) {
        return false;
      }
    } else {
      if (E.getKind() != K) {
        return false;
      }
    }
    auto N = E.getNumChildren();
    for (auto Pair : Children) {
      auto CNum = Pair.first;
      auto Pat = Pair.second;
      if (CNum >= N)
        return false;
      if (!Pat.Match(E.getChildren()[CNum]))
        return false;
    }
    return true;
  }
private:
  CVC4::Type T;
  CVC4::Kind K;
  std::map<int, Pattern> Children;
  bool isTypePattern;
};

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
}
#endif
