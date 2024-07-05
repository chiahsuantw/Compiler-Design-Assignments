#include "AST/if.hpp"

void IfNode::visitChildNodes(AstNodeVisitor &visitor) {
  expr->accept(visitor);
  if_body->accept(visitor);
  if (else_body) {
    else_body->accept(visitor);
  }
}
