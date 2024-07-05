#ifndef __AST_FUNCTION_INVOCATION_NODE_H
#define __AST_FUNCTION_INVOCATION_NODE_H

#include "AST/expression.hpp"
#include "visitor/AstNodeVisitor.hpp"

#include <memory>
#include <string>
#include <vector>

class FunctionInvocationNode : public ExpressionNode {
  using ExpressionNodes = std::vector<std::unique_ptr<ExpressionNode>>;

public:
  FunctionInvocationNode(uint32_t line, uint32_t col, char *name,
                         ExpressionNodes &exprs)
      : ExpressionNode{line, col}, name(name), exprs(std::move(exprs)){};

  const char *getNameCString() const { return name.c_str(); }

  void accept(AstNodeVisitor &visitor) override { visitor.visit(*this); };
  void visitChildNodes(AstNodeVisitor &visitor) override;

private:
  std::string name;
  ExpressionNodes exprs;
};

#endif
