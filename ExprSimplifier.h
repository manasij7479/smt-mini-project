#ifndef MM_SIMP_H
#define MM_SIMP_H
#include <cvc4/cvc4.h>
#include "ExprMatcher.h"
namespace mm {

std::pair<CVC4::Expr, bool> ShortCircuitBinaryAnd(CVC4::SmtEngine &SMT, CVC4::Expr E);
std::pair<CVC4::Expr, bool> ShortCircuitBinaryOr(CVC4::SmtEngine &SMT, CVC4::Expr E);
std::pair<CVC4::Expr, bool> SubsumeBinaryAnd(CVC4::SmtEngine &SMT, CVC4::Expr E);

std::pair<CVC4::Expr, bool> SubsumeBinaryOr(CVC4::SmtEngine &SMT, CVC4::Expr E);

std::pair<CVC4::Expr, bool> SimplifyImplication(CVC4::SmtEngine &SMT, CVC4::Expr E);

CVC4::Expr ApplyAllRecursively(CVC4::SmtEngine &SMT, CVC4::Expr E);
 
}
#endif
