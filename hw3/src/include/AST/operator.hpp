#ifndef __AST_OPERATOR_NODE_H
#define __AST_OPERATOR_NODE_H

enum class Operator {
  UNARY_MINUS,
  MULTIPLY,
  DIVIDE,
  MOD,
  PLUS,
  MINUS,
  LESS,
  LESS_OR_EQUAL,
  GREATER,
  GREATER_OR_EQUAL,
  EQUAL,
  NOT_EQUAL,
  NOT,
  AND,
  OR
};

extern const char *opStrings[];

#endif
