#ifndef __AST_ASSIGNMENT_NODE_H
#define __AST_ASSIGNMENT_NODE_H

#include "AST/VariableReference.hpp"
#include "AST/ast.hpp"
#include "visitor/AstNodeVisitor.hpp"

#include <memory>

class AssignmentNode : public AstNode {
public:
  AssignmentNode(uint32_t line, uint32_t col, VariableReferenceNode *varRef,
                 ExpressionNode *expr)
      : AstNode{line, col}, varRef(varRef), expr(expr){};

  void accept(AstNodeVisitor &visitor) override { visitor.visit(*this); };
  void visitChildNodes(AstNodeVisitor &visitor) override;

private:
  std::unique_ptr<VariableReferenceNode> varRef;
  std::unique_ptr<ExpressionNode> expr;
};

#endif
