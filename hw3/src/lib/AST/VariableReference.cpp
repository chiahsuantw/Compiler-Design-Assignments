#include "AST/VariableReference.hpp"

void VariableReferenceNode::visitChildNodes(AstNodeVisitor &visitor) {
  for (auto &expr : exprs)
    expr->accept(visitor);
}
