#include <iostream>
#include <cvc4/cvc4.h>
using namespace CVC4;
int main() {
  ExprManager em;
  Expr helloworld = em.mkVar("Hello World!", em.booleanType());
  SmtEngine smt(&em);
  std::cout << helloworld << " is " << smt.query(helloworld) << std::endl;
  return 0;
}
