#ifndef MM_SUMMARY_H
#define MM_SUMMARY_H
#include <cvc4/cvc4.h>
#include <unordered_map>
namespace mm {
  typedef std::unordered_map<std::string, CVC4::Expr> Table;
  class Summary {
  public:
    void dump(std::ostream &Out, CVC4::SmtEngine &SMT) {
      for (auto Pair : SMap) {
        Out << Pair.first << " -> " << Pair.second.toString() << std::endl;
      }
      Out << "Pred : " << getPredicate(SMT).toString() << std::endl << std::endl;
    }
    void substitute(std::string Var, CVC4::Expr E, Table &Vars) {
//       std::cout << "HERE: " << Var << '\t' << E.toString()  << "\t" << Predicate.toString() << std::endl;
      if (SMap.find(Var) == SMap.end()) {
        SMap[Var] = Vars[Var];
      }
//       SMap[Var] =  SMap[Var].substitute(Vars[Var], E);
      for (auto Pair : SMap) {
        SMap[Pair.first] = SMap[Pair.first].substitute(Vars[Var], E);
      }
      Predicate = Predicate.substitute(Vars[Var], E);
    }
    void addPredicate(CVC4::Expr E, CVC4::SmtEngine &SMT, Table &Vars) {
//       Predicate = getPredicate(SMT).andExpr(apply(E, Vars));
      Predicate = getPredicate(SMT).andExpr(E);
    }
    CVC4::Expr apply(CVC4::Expr E ,Table &Vars) {
      std::vector<CVC4::Expr> Pre, Post;
      for (auto Pair : SMap) {
        Pre.push_back(Vars[Pair.first]);
        Post.push_back(Pair.second);
      }
      return E.substitute(Pre, Post);
    }
    CVC4::Expr getPredicate(CVC4::SmtEngine &SMT);
    void branch(CVC4::Expr Cond, Summary T, Summary F, CVC4::SmtEngine &SMT, Table &Vars) {
//       Cond = apply(Cond, Vars);
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
