#include "AST/decl.hpp"

void DeclNode::visitChildNodes(AstNodeVisitor &visitor) {
  for (auto &var : varNodes)
    var->accept(visitor);
}
