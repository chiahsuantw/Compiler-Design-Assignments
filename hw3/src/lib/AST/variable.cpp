#include "AST/variable.hpp"

void VariableNode::visitChildNodes(AstNodeVisitor &visitor) {
  if (value)
    value->accept(visitor);
}
