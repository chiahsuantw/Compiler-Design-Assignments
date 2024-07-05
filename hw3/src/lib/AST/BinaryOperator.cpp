#include "AST/BinaryOperator.hpp"

void BinaryOperatorNode::visitChildNodes(AstNodeVisitor &visitor) {
  left_expr->accept(visitor);
  right_expr->accept(visitor);
}
