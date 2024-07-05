#ifndef __AST_TYPE_NODE_H
#define __AST_TYPE_NODE_H

#include <string>
#include <vector>

class Type {
public:
  enum class TypeEnum {
    VOID,
    INTEGER,
    REAL,
    BOOLEAN,
    STRING,
  };

  const char *typeStrings[5] = {"void", "integer", "real", "boolean", "string"};

  TypeEnum type;
  std::vector<int> dims;
  mutable std::string typeString;

  Type(const TypeEnum type) : type(type) {}

  void setDimensions(std::vector<int> &dims) { this->dims = std::move(dims); }

  const char *getTypeCString() const {
    if (typeString.empty()) {
      typeString = typeStrings[static_cast<int>(type)];
      if (!dims.empty()) {
        typeString += " ";
        for (const auto &dim : dims) {
          typeString += "[" + std::to_string(dim) + "]";
        }
      }
    }
    return typeString.c_str();
  }
};

#endif
