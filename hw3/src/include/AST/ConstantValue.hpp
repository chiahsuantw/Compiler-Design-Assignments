#ifndef __AST_CONSTANT_VALUE_NODE_H
#define __AST_CONSTANT_VALUE_NODE_H

#include "AST/constant.hpp"
#include "AST/expression.hpp"
#include "AST/type.hpp"
#include "visitor/AstNodeVisitor.hpp"

class ConstantValueNode : public ExpressionNode {
public:
  ConstantValueNode(const uint32_t line, const uint32_t col,
                    Constant *const constant)
      : ExpressionNode{line, col}, constant(constant){};

  std::shared_ptr<Type> getType() const { return constant->getType(); }
  const char *getValueCString() const { return constant->getValueCString(); }

  void accept(AstNodeVisitor &visitor) override { visitor.visit(*this); };

private:
  std::unique_ptr<Constant> constant;
};

#endif
