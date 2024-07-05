#include "AST/program.hpp"

void ProgramNode::visitChildNodes(AstNodeVisitor &visitor) {
  for (auto &decl : declNodes)
    decl->accept(visitor);
  for (auto &func : funcNodes)
    func->accept(visitor);
  body->accept(visitor);
}
