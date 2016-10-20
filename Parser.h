#ifndef MM_PARSER_H
#define MM_PARSER_H

#include "ParserUtils.h"

namespace mm {
Result ParseExpr(Stream in);
Result ParseIntVar(Stream in);
Result ParseBVVar(Stream in);
Result ParseVar(Stream in);
Result ParseIntConst(Stream in);
Result ParseBVConst(Stream in);
Result ParseBoolConst(Stream in);
Result ParseBinaryExpr(Stream in);
Result ParseUnaryExpr(Stream in);
Result ParseUFExpr(Stream in);
Result ParseNestedExpr(Stream in);
Result ParseAssignStmt(Stream in);
Result ParseSeqStmt(Stream in);
Result ParseCondStmt(Stream in);
Result ParseStmt(Stream in);
Result ParseProgram(Stream in);

}
#endif
