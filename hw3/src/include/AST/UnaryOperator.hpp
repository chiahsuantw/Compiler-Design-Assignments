#ifndef __AST_UNARY_OPERATOR_NODE_H
#define __AST_UNARY_OPERATOR_NODE_H

#include "AST/expression.hpp"
#include "AST/operator.hpp"
#include "visitor/AstNodeVisitor.hpp"

#include <memory>

class UnaryOperatorNode : public ExpressionNode {
public:
  UnaryOperatorNode(uint32_t line, uint32_t col, Operator op,
                    ExpressionNode *expr)
      : ExpressionNode{line, col}, op(op), expr(expr){};

  const char *getOpCString() const { return opStrings[static_cast<int>(op)]; }

  void accept(AstNodeVisitor &visitor) override { visitor.visit(*this); };
  void visitChildNodes(AstNodeVisitor &visitor) override;

private:
  Operator op;
  std::unique_ptr<ExpressionNode> expr;
};

#endif
