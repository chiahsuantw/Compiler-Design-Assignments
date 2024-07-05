#include "AST/function.hpp"

const char *FunctionNode::getPrototypeCString() const {
  if (prototypeString.empty()) {
    prototypeString = type->getTypeCString();
    prototypeString += " (";
    for (auto &decl : declNodes) {
      for (auto &var : decl->varNodes) {
        prototypeString += var->getTypeCString();
        prototypeString += ", ";
      }
    }
    if (!declNodes.empty())
      prototypeString.erase(prototypeString.end() - 2, prototypeString.end());
    prototypeString += ")";
  }
  return prototypeString.c_str();
}

void FunctionNode::visitChildNodes(AstNodeVisitor &visitor) {
  for (auto &decl : declNodes)
    decl->accept(visitor);
  if (body)
    body->accept(visitor);
}
