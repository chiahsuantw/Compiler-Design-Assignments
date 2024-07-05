#ifndef AST_PROGRAM_NODE_H
#define AST_PROGRAM_NODE_H

#include "AST/CompoundStatement.hpp"
#include "AST/ast.hpp"
#include "AST/decl.hpp"
#include "AST/function.hpp"
#include "visitor/AstNodeVisitor.hpp"

#include <memory>
#include <string>
#include <vector>

class ProgramNode final : public AstNode {
  using DeclarationNodes = std::vector<std::unique_ptr<DeclNode>>;
  using FunctionNodes = std::vector<std::unique_ptr<FunctionNode>>;

private:
  std::string name;
  std::unique_ptr<Type> type;
  DeclarationNodes declNodes;
  FunctionNodes funcNodes;
  std::unique_ptr<CompoundStatementNode> body;

public:
  ProgramNode(uint32_t line, uint32_t col, char *name, Type *type,
              DeclarationNodes &declNodes, FunctionNodes &funcNodes,
              CompoundStatementNode *body)
      : AstNode{line, col}, name(name), type(type),
        declNodes(std::move(declNodes)), funcNodes(std::move(funcNodes)),
        body(body){};

  const char *getNameCString() const { return name.c_str(); };

  void accept(AstNodeVisitor &visitor) override { visitor.visit(*this); };
  void visitChildNodes(AstNodeVisitor &visitor) override;
};

#endif
