#ifndef __AST_ID_NODE_H
#define __AST_ID_NODE_H

#include "AST/ast.hpp"

#include <string>

class IdNode {
public:
  Location location;
  std::string name;

  IdNode(const uint32_t line, const uint32_t col, const char *const name)
      : location(line, col), name(name){};
};

#endif
