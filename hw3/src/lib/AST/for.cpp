#include "AST/for.hpp"

void ForNode::visitChildNodes(AstNodeVisitor &visitor) {
  decl->accept(visitor);
  asgmt->accept(visitor);
  expr->accept(visitor);
  body->accept(visitor);
}
