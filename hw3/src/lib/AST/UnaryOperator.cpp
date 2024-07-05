#include "AST/UnaryOperator.hpp"

void UnaryOperatorNode::visitChildNodes(AstNodeVisitor &visitor) {
  expr->accept(visitor);
}
