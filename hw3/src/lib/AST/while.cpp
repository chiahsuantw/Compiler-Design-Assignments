#include "AST/while.hpp"

void WhileNode::visitChildNodes(AstNodeVisitor &visitor) {
  expr->accept(visitor);
  body->accept(visitor);
}
