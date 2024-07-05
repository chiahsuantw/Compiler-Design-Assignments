#include "AST/read.hpp"

void ReadNode::visitChildNodes(AstNodeVisitor &visitor) {
  varRef->accept(visitor);
}
