#ifndef __AST_COMPOUND_STATEMENT_NODE_H
#define __AST_COMPOUND_STATEMENT_NODE_H

#include "AST/ast.hpp"
#include "AST/decl.hpp"
#include "visitor/AstNodeVisitor.hpp"

#include <memory>
#include <vector>

class CompoundStatementNode : public AstNode {
  using DeclarationNodes = std::vector<std::unique_ptr<DeclNode>>;
  using StatementNodes = std::vector<std::unique_ptr<AstNode>>;

public:
  CompoundStatementNode(uint32_t line, uint32_t col,
                        DeclarationNodes &declNodes, StatementNodes &stmtNodes)
      : AstNode{line, col}, declNodes{std::move(declNodes)},
        stmtNodes{std::move(stmtNodes)} {};

  void accept(AstNodeVisitor &visitor) override { visitor.visit(*this); };
  void visitChildNodes(AstNodeVisitor &visitor) override;

private:
  DeclarationNodes declNodes;
  StatementNodes stmtNodes;
};

#endif
