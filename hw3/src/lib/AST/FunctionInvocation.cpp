#include "AST/FunctionInvocation.hpp"

void FunctionInvocationNode::visitChildNodes(AstNodeVisitor &visitor) {
  for (auto &expr : exprs)
    expr->accept(visitor);
}
