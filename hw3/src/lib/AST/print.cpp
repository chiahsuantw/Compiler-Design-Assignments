#include "AST/print.hpp"

void PrintNode::visitChildNodes(AstNodeVisitor &visitor) {
  expr->accept(visitor);
}
