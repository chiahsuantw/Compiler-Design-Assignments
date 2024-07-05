#ifndef AST_AST_NODE_H
#define AST_AST_NODE_H

#include <cstdint>

class AstNodeVisitor;

struct Location {
  uint32_t line;
  uint32_t col;

  Location(const uint32_t line, const uint32_t col) : line(line), col(col) {}
};

class AstNode {
protected:
  Location location;

public:
  virtual ~AstNode() = 0;
  AstNode(const uint32_t line, const uint32_t col) : location(line, col){};

  AstNode(const AstNode &) = delete;
  AstNode(AstNode &&) = delete;
  AstNode &operator=(const AstNode &) = delete;
  AstNode &operator=(AstNode &&) = delete;

  const Location &getLocation() const { return location; };

  virtual void accept(AstNodeVisitor &) = 0;
  virtual void visitChildNodes(AstNodeVisitor &) {};
};

#endif
