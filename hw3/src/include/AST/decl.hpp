#ifndef __AST_DECL_NODE_H
#define __AST_DECL_NODE_H

#include "AST/ConstantValue.hpp"
#include "AST/Id.hpp"
#include "AST/ast.hpp"
#include "AST/decl.hpp"
#include "AST/type.hpp"
#include "AST/variable.hpp"
#include "visitor/AstNodeVisitor.hpp"

#include <iostream>
#include <memory>
#include <vector>

class DeclNode : public AstNode {
  using VariableNodes = std::vector<std::unique_ptr<VariableNode>>;

public:
  VariableNodes varNodes;

  DeclNode(uint32_t line, uint32_t col, std::vector<IdNode> *ids, Type *type)
      : AstNode{line, col} {
    std::shared_ptr<Type> shared_type{type};
    std::shared_ptr<ConstantValueNode> shared_value{nullptr};
    for (auto &id : *ids) {
      varNodes.emplace_back(new VariableNode(id.location.line, id.location.col,
                                             id.name, shared_type,
                                             shared_value));
    }
  };

  DeclNode(uint32_t line, uint32_t col, std::vector<IdNode> *ids,
           ConstantValueNode *value)
      : AstNode{line, col} {
    std::shared_ptr<Type> shared_type{value->getType()};
    std::shared_ptr<ConstantValueNode> shared_value{value};
    for (auto &id : *ids) {
      varNodes.emplace_back(new VariableNode(id.location.line, id.location.col,
                                             id.name, shared_type,
                                             shared_value));
    }
  };

  void accept(AstNodeVisitor &visitor) override { visitor.visit(*this); };
  void visitChildNodes(AstNodeVisitor &visitor) override;
};

#endif
