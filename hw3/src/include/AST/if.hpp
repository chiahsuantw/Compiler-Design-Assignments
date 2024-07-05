#ifndef __AST_IF_NODE_H
#define __AST_IF_NODE_H

#include "AST/CompoundStatement.hpp"
#include "AST/ast.hpp"
#include "AST/expression.hpp"
#include "visitor/AstNodeVisitor.hpp"

#include <memory>

class IfNode : public AstNode {
public:
  IfNode(uint32_t line, uint32_t col, ExpressionNode *expr,
         CompoundStatementNode *if_body, CompoundStatementNode *else_body)
      : AstNode{line, col}, expr(expr), if_body(if_body),
        else_body(else_body){};

  void accept(AstNodeVisitor &visitor) override { visitor.visit(*this); };
  void visitChildNodes(AstNodeVisitor &visitor) override;

private:
  std::unique_ptr<ExpressionNode> expr;
  std::unique_ptr<CompoundStatementNode> if_body;
  std::unique_ptr<CompoundStatementNode> else_body;
};

#endif
