#ifndef __AST_READ_NODE_H
#define __AST_READ_NODE_H

#include "AST/VariableReference.hpp"
#include "AST/ast.hpp"
#include "visitor/AstNodeVisitor.hpp"

#include <memory>

class ReadNode : public AstNode {
public:
  ReadNode(uint32_t line, uint32_t col, VariableReferenceNode *varRef)
      : AstNode{line, col}, varRef(varRef){};

  void accept(AstNodeVisitor &visitor) override { visitor.visit(*this); };
  void visitChildNodes(AstNodeVisitor &visitor) override;

private:
  std::unique_ptr<VariableReferenceNode> varRef;
};

#endif
