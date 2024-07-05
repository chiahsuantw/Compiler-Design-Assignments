#ifndef __AST_BINARY_OPERATOR_NODE_H
#define __AST_BINARY_OPERATOR_NODE_H

#include "AST/expression.hpp"
#include "AST/operator.hpp"
#include "visitor/AstNodeVisitor.hpp"

#include <memory>

class BinaryOperatorNode : public ExpressionNode {
public:
  BinaryOperatorNode(uint32_t line, uint32_t col, Operator op,
                     ExpressionNode *left_expr, ExpressionNode *right_expr)
      : ExpressionNode{line, col}, op(op), left_expr(left_expr),
        right_expr(right_expr){};

  const char *getOpCString() const { return opStrings[static_cast<int>(op)]; }

  void accept(AstNodeVisitor &visitor) override { visitor.visit(*this); };
  void visitChildNodes(AstNodeVisitor &visitor) override;

private:
  Operator op;
  std::unique_ptr<ExpressionNode> left_expr;
  std::unique_ptr<ExpressionNode> right_expr;
};

#endif
