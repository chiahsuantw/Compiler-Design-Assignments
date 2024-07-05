#ifndef __AST_FUNCTION_NODE_H
#define __AST_FUNCTION_NODE_H

#include "AST/CompoundStatement.hpp"
#include "AST/ast.hpp"
#include "AST/decl.hpp"
#include "AST/type.hpp"
#include "visitor/AstNodeVisitor.hpp"

#include <memory>
#include <string>
#include <vector>

class FunctionNode : public AstNode {
  using DeclarationNodes = std::vector<std::unique_ptr<DeclNode>>;

public:
  FunctionNode(uint32_t line, uint32_t col, char *name,
               DeclarationNodes &declNodes, Type *type,
               CompoundStatementNode *body)
      : AstNode{line, col}, name(name), declNodes(std::move(declNodes)),
        type(type), body(body){};

  const char *getNameCString() const { return name.c_str(); };
  const char *getPrototypeCString() const;

  void accept(AstNodeVisitor &visitor) override { visitor.visit(*this); };
  void visitChildNodes(AstNodeVisitor &visitor) override;

private:
  std::string name;
  DeclarationNodes declNodes;
  std::unique_ptr<Type> type;
  std::unique_ptr<CompoundStatementNode> body;

  mutable std::string prototypeString;
};

#endif
