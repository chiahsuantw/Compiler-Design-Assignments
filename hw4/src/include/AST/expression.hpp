#ifndef AST_EXPRESSION_NODE_H
#define AST_EXPRESSION_NODE_H

#include "AST/ast.hpp"
#include "AST/PType.hpp"

#include <memory>

class ExpressionNode : public AstNode {
  public:
    ~ExpressionNode() = default;
    ExpressionNode(const uint32_t line, const uint32_t col)
        : AstNode{line, col} {}
    PType *getType() { return type.get(); }
    void setType(PType *t) { type.reset(t); }

  protected:
    // for carrying type of result of an expression
    std::unique_ptr<PType> type;
};

#endif
