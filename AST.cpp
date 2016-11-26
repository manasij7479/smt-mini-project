#include "AST.h"
bool SIMP_COND = false;
std::unordered_map<std::string, mm::DefStmt *>& FDEFS() { //TODO : Remove global
  static std::unordered_map<std::string, mm::DefStmt *> FDEFS_;
  return FDEFS_;
}
std::unordered_map<std::string, mm::Summary>& SUMMARIES() {
  static bool first = true;
  static std::unordered_map<std::string, mm::Summary> *FOO;
  if (first) {
    first = false;
    FOO = new std::unordered_map<std::string, mm::Summary>();
  }
  return *FOO;
}
namespace mm {
CVC4::Expr BinaryExpr::Translate(CVC4::ExprManager &EM, std::unordered_map<std::string, CVC4::Expr> &VARS) {
  std::unordered_map<std::string, CVC4::Kind> Map = {
    {"+", CVC4::Kind::PLUS},
    {"-", CVC4::Kind::MINUS},
    {"->", CVC4::Kind::IMPLIES},
    {"*", CVC4::Kind::MULT},
    {"/", CVC4::Kind::DIVISION},
    {"&&", CVC4::Kind::AND},
    {"||", CVC4::Kind::OR},
    {"==", CVC4::Kind::EQUAL},
    {"<=", CVC4::Kind::LEQ},
    {">=", CVC4::Kind::GEQ},
    {"<", CVC4::Kind::LT},
    {">", CVC4::Kind::GT},
    {"!", CVC4::Kind::NOT},
    {"&", CVC4::Kind::BITVECTOR_AND},
    {"|", CVC4::Kind::BITVECTOR_OR},
    {"'>", CVC4::Kind::BITVECTOR_UGT},
    {"'>=", CVC4::Kind::BITVECTOR_UGE},
    {"'<", CVC4::Kind::BITVECTOR_ULT},
    {"'<=", CVC4::Kind::BITVECTOR_ULE},
  };
  return EM.mkExpr(Map[Op], Left->Translate(EM, VARS), Right->Translate(EM, VARS));
}

CVC4::Expr UnaryExpr::Translate(CVC4::ExprManager &EM, std::unordered_map<std::string, CVC4::Expr> &VARS) {
  std::unordered_map<std::string, CVC4::Kind> Map = {
    {"-", CVC4::Kind::UMINUS},
    {"!", CVC4::Kind::NOT},
    {"~", CVC4::Kind::BITVECTOR_NOT}
  };
  return EM.mkExpr(Map[Op], SubExpr->Translate(EM, VARS));
}

void SeqStmt::dump(std::ostream &Out, int level) {
  tab(Out, level);
  Out << "{\n";
  for (auto *Simple : Statements) {
    Simple->dump(Out, level+1);
  }
  tab(Out, level);
  Out << "}\n";
}

void CondStmt::dump(std::ostream &Out, int level) {
  tab(Out, level);
  Out << "if ( ";
  Condition->dump(Out);
  Out << " ) { \n";
  TrueStmt->dump(Out, level+1);
  tab(Out, level);
  Out << "} else {\n";
  FalseStmt->dump(Out, level+1);
  tab(Out, level);
  Out << "}\n";
}

void Program::dump(std::ostream &Out) {
  Out << "pre:\t";
  Pre->dump(Out);
  Out << "\nstmt:\n";
  Statement->dump(Out, 0);
  Out << "post:\t";
  Post->dump(Out);
  Out << std::endl;
}
}
