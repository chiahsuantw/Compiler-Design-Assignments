#ifndef __AST_FOR_NODE_H
#define __AST_FOR_NODE_H

#include "AST/CompoundStatement.hpp"
#include "AST/assignment.hpp"
#include "AST/ast.hpp"
#include "AST/decl.hpp"
#include "AST/expression.hpp"
#include "visitor/AstNodeVisitor.hpp"

#include <memory>

class ForNode : public AstNode {
public:
  ForNode(const uint32_t line, const uint32_t col, DeclNode *decl,
          AssignmentNode *asgmt, ExpressionNode *expr,
          CompoundStatementNode *body)
      : AstNode{line, col}, decl(decl), asgmt(asgmt), expr(expr), body(body){};

  void accept(AstNodeVisitor &visitor) override { visitor.visit(*this); };
  void visitChildNodes(AstNodeVisitor &visitor) override;

private:
  std::unique_ptr<DeclNode> decl;
  std::unique_ptr<AssignmentNode> asgmt;
  std::unique_ptr<ExpressionNode> expr;
  std::unique_ptr<CompoundStatementNode> body;
};

#endif
