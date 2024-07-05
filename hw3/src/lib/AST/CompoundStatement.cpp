#include "AST/CompoundStatement.hpp"

void CompoundStatementNode::visitChildNodes(AstNodeVisitor &visitor) {
  for (auto &decl : declNodes)
    decl->accept(visitor);
  for (auto &stmt : stmtNodes)
    stmt->accept(visitor);
}
