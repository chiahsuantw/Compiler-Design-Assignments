#ifndef __AST_CONSTANT_NODE_H
#define __AST_CONSTANT_NODE_H

#include "AST/type.hpp"

#include <memory>
#include <string>

class Constant {
public:
  union ConstantValue {
    int integer;
    double real;
    char *string;
    bool boolean;
  };

  std::shared_ptr<Type> type;
  ConstantValue value;

  Constant(const std::shared_ptr<Type> &type, const ConstantValue value)
      : type(type), value(value) {}

  std::shared_ptr<Type> getType() const { return type; }
  const char *getValueCString() const {
    switch (type->type) {
    case Type::TypeEnum::VOID:
      return "";
    case Type::TypeEnum::INTEGER:
      return std::to_string(value.integer).c_str();
    case Type::TypeEnum::REAL:
      return std::to_string(value.real).c_str();
    case Type::TypeEnum::STRING:
      return value.string;
    case Type::TypeEnum::BOOLEAN:
      return value.boolean ? "true" : "false";
    }
    return "";
  }
};

#endif
