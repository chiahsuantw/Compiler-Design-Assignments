#ifndef __AST_PRINT_NODE_H
#define __AST_PRINT_NODE_H

#include "AST/ast.hpp"
#include "AST/expression.hpp"
#include "visitor/AstNodeVisitor.hpp"

#include <memory>

class PrintNode : public AstNode {
public:
  PrintNode(uint32_t line, uint32_t col, ExpressionNode *expr)
      : AstNode{line, col}, expr(expr){};

  void accept(AstNodeVisitor &visitor) override { visitor.visit(*this); };
  void visitChildNodes(AstNodeVisitor &visitor) override;

private:
  std::unique_ptr<ExpressionNode> expr;
};

#endif
