#include "Summary.h"
#include "ExprSimplifier.h"
namespace mm {
  CVC4::Expr Summary::getPredicate(CVC4::SmtEngine &SMT) {
    if (Predicate.isNull())
      return SMT.getExprManager()->mkConst(true);
    return ApplyAllRecursively(SMT, Predicate);
  }
}
