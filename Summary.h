#ifndef MM_SUMMARY_H
#define MM_SUMMARY_H
#include <cvc4/cvc4.h>
#include <unordered_map>
namespace mm {
  typedef std::unordered_map<std::string, CVC4::Expr> Table;
  class Summary {
  public:
    void dump(std::ostream &Out) {
      for (auto Pair : SMap) {
        Out << Pair.first << " -> " << Pair.second.toString() << std::endl;
      }
      Out << "Pred : " << Predicate.toString() << std::endl;
    }
    void substitute(std::string Var, CVC4::Expr E, Table &Vars) {
      if (SMap.find(Var) == SMap.end()) {
        SMap[Var] = Vars[Var];
      }
      SMap[Var] =  SMap[Var].substitute(Vars[Var], E);
    }
    void addPredicate(CVC4::Expr E, CVC4::SmtEngine &SMT, Table &Vars) {
      Predicate = getPredicate(SMT).andExpr(apply(E, Vars));
    }
    CVC4::Expr apply(CVC4::Expr E ,Table &Vars) {
      auto Result = E;
      for (auto Pair : SMap) {
        Result = Result.substitute(Vars[Pair.first], Pair.second);
      }
      return Result;
    }
    CVC4::Expr getPredicate(CVC4::SmtEngine &SMT) {
      if (Predicate.isNull())
        return SMT.getExprManager()->mkConst(true);
      return Predicate;
    }
    void branch(CVC4::Expr Cond, Summary T, Summary F, CVC4::SmtEngine &SMT, Table &Vars) {
      Predicate = getPredicate(SMT).andExpr(Cond.impExpr(T.getPredicate(SMT)));
      Predicate = getPredicate(SMT).andExpr(Cond.notExpr().impExpr(F.getPredicate(SMT)));
      
      std::unordered_map<std::string, CVC4::Expr> PreSMap;
      for (auto Pair : T.SMap) {
        CVC4::Expr TE = Pair.second;
        CVC4::Expr FE = Vars[Pair.first];
        if (F.SMap.find(Pair.first) != F.SMap.end()) {
          FE = F.SMap[Pair.first];
        }
        PreSMap[Pair.first] = Cond.iteExpr(TE, FE);
      }
      for (auto Pair : F.SMap) {
        if (PreSMap.find(Pair.first) != PreSMap.end())
          break;
        CVC4::Expr FE = Pair.second;
        CVC4::Expr TE = Vars[Pair.first];
        if (T.SMap.find(Pair.first) != T.SMap.end()) {
          TE = T.SMap[Pair.first];
        }
        PreSMap[Pair.first] = Cond.iteExpr(TE, FE);
      }
      for (auto Pair : PreSMap) {
        substitute(Pair.first, Pair.second, Vars);
      }
    }
  private:
    std::unordered_map<std::string, CVC4::Expr> SMap; // Initially identity
    CVC4::Expr Predicate;
  };
}
#endif
