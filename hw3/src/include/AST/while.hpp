#ifndef __AST_WHILE_NODE_H
#define __AST_WHILE_NODE_H

#include "AST/CompoundStatement.hpp"
#include "AST/ast.hpp"
#include "AST/expression.hpp"
#include "visitor/AstNodeVisitor.hpp"

#include <memory>

class WhileNode : public AstNode {
public:
  WhileNode(uint32_t line, uint32_t col, ExpressionNode *expr,
            CompoundStatementNode *body)
      : AstNode{line, col}, expr(expr), body(body){};

  void accept(AstNodeVisitor &visitor) override { visitor.visit(*this); };
  void visitChildNodes(AstNodeVisitor &visitor) override;

private:
  std::unique_ptr<ExpressionNode> expr;
  std::unique_ptr<CompoundStatementNode> body;
};

#endif
