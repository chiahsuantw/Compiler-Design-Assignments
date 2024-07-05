#include "AST/assignment.hpp"

void AssignmentNode::visitChildNodes(AstNodeVisitor &visitor) {
  varRef->accept(visitor);
  expr->accept(visitor);
}
