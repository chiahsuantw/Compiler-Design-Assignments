#ifndef __AST_VARIABLE_NODE_H
#define __AST_VARIABLE_NODE_H

#include "AST/ConstantValue.hpp"
#include "AST/ast.hpp"
#include "AST/type.hpp"
#include "visitor/AstNodeVisitor.hpp"

#include <memory>
#include <string>

class VariableNode : public AstNode {
public:
  VariableNode(const uint32_t line, const uint32_t col, const std::string name,
               const std::shared_ptr<Type> &type,
               const std::shared_ptr<ConstantValueNode> &value)
      : AstNode{line, col}, name(name), type(type), value(value){};

  const char *getNameCString() const { return name.c_str(); };
  const char *getTypeCString() const { return type->getTypeCString(); }

  void accept(AstNodeVisitor &visitor) override { visitor.visit(*this); };
  void visitChildNodes(AstNodeVisitor &visitor) override;

private:
  std::string name;
  std::shared_ptr<Type> type;
  std::shared_ptr<ConstantValueNode> value;
};

#endif
