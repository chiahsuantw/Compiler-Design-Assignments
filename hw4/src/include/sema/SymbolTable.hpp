#ifndef SEMA_SYMBOL_TABLE_H
#define SEMA_SYMBOL_TABLE_H

#include "AST/PType.hpp"
#include "AST/constant.hpp"
#include "AST/function.hpp"

#include <stack>
#include <string>
#include <vector>

class SymbolEntry {
  public:
    enum Kind { PROGRAM, FUNCTION, PARAMETER, VARIABLE, LOOP_VAR, CONSTANT };
    enum AttrKind { CONST_VAL, PARAMS };
    std::string name;
    Kind kind;
    int level;
    PType *type;
    AttrKind attrKind;
    union {
        Constant *const_val_ptr;
        FunctionNode::DeclNodes *params_ptr;
    };
    SymbolEntry(std::string name, Kind kind, int level, PType *type,
                Constant *const_val_ptr)
        : name(name), kind(kind), level(level), type(type), attrKind(CONST_VAL),
          const_val_ptr(const_val_ptr) {}
    SymbolEntry(std::string name, Kind kind, int level, PType *type,
                FunctionNode::DeclNodes *params_ptr)
        : name(name), kind(kind), level(level), type(type), attrKind(PARAMS),
          params_ptr(params_ptr) {}
    std::string getKindString() {
      switch (kind) {
      case PROGRAM:
        return "program";
      case FUNCTION:
        return "function";
      case PARAMETER:
        return "parameter";
      case VARIABLE:
        return "variable";
      case LOOP_VAR:
        return "loop_var";
      case CONSTANT:
        return "constant";
      }
      return "";
    }
    std::string getAttrString() {
      switch (attrKind) {
      case CONST_VAL:
        return const_val_ptr ? const_val_ptr->getConstantValueCString() : "";
      case PARAMS:
        return FunctionNode::getParametersTypeString(*params_ptr).c_str();
      }
      return "";
    }
};

class SymbolTable {
  public:
    std::vector<SymbolEntry> entries;
    void addSymbol(std::string name, SymbolEntry::Kind kind, int level,
                   PType *type, Constant *const_val_ptr) {
      entries.push_back(SymbolEntry(name, kind, level, type, const_val_ptr));
    }
    void addSymbol(std::string name, SymbolEntry::Kind kind, int level,
                   PType *type, FunctionNode::DeclNodes *params_ptr) {
      entries.push_back(SymbolEntry(name, kind, level, type, params_ptr));
    }
    SymbolEntry *findSymbol(std::string name) {
      for (auto &entry : entries)
        if (entry.name == name)
          return &entry;
      return nullptr;
    }
};

class SymbolManager {
  public:
    SymbolTable *cur_table = nullptr;
    int cur_level = -1;
    std::stack<SymbolTable *> tables;
    void pushScope(SymbolTable *table);
    void popScope();
    void dumpSymbolTable(SymbolTable *table);
};

#endif // SEMA_SYMBOL_TABLE_H
