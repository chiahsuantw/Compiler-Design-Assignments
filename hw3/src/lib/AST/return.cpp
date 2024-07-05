#include "AST/return.hpp"

void ReturnNode::visitChildNodes(AstNodeVisitor &visitor) {
  expr->accept(visitor);
}
