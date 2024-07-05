#ifndef SEMA_SYMBOL_TABLE_H
#define SEMA_SYMBOL_TABLE_H

#include <cstddef>
#include <cstdint>
#include <memory>
#include <string>
#include <vector>

#include "AST/PType.hpp"
#include "AST/constant.hpp"
#include "AST/function.hpp"

/*
 * Conform to C++ Core Guidelines C.182
 */
class Attribute {
  private:
    enum class Tag { kConstantValue, kParameterDeclNodes };
    Tag m_type;

    union {
        // raw pointer, does not own the object
        const Constant *m_constant_value_ptr;
        const FunctionNode::DeclNodes *m_parameters_ptr;
    };

  public:
    ~Attribute() = default;

    Attribute(const Constant *p_constant)
        : m_type(Tag::kConstantValue), m_constant_value_ptr(p_constant) {}

    Attribute(const FunctionNode::DeclNodes *p_parameters)
        : m_type(Tag::kParameterDeclNodes), m_parameters_ptr(p_parameters) {}

    const Constant *constant() const;
    const FunctionNode::DeclNodes *parameters() const;
};

class SymbolEntry {
  public:
    enum class KindEnum : uint8_t {
        kProgramKind,
        kFunctionKind,
        kParameterKind,
        kVariableKind,
        kLoopVarKind,
        kConstantKind
    };

  private:
    const std::string &m_name;
    KindEnum m_kind;
    size_t m_level;
    const PType *m_p_type;
    Attribute m_attribute;
    int m_offset;

  public:
    ~SymbolEntry() = default;

    SymbolEntry(const std::string &p_name, const KindEnum p_kind,
                const size_t p_level, const PType *const p_p_type,
                const Constant *const p_constant, const int p_offset)
        : m_name(p_name),
          m_kind(p_kind),
          m_level(p_level),
          m_p_type(p_p_type),
          m_attribute(p_constant),
          m_offset(p_offset) {}

    SymbolEntry(const std::string &p_name, const KindEnum p_kind,
                const size_t p_level, const PType *const p_p_type,
                const FunctionNode::DeclNodes *const p_parameters,
                const int p_offset)
        : m_name(p_name),
          m_kind(p_kind),
          m_level(p_level),
          m_p_type(p_p_type),
          m_attribute(p_parameters),
          m_offset(p_offset) {}

    const std::string &getName() const { return m_name; }
    const char *getNameCString() const { return m_name.c_str(); }

    const KindEnum getKind() const { return m_kind; }

    const size_t getLevel() const { return m_level; }

    const PType *getTypePtr() const { return m_p_type; }

    const Attribute &getAttribute() const { return m_attribute; }

    const int getOffset() const { return m_offset; }
};

class SymbolTable {
  private:
    std::vector<std::unique_ptr<SymbolEntry>> m_entries;

  public:
    ~SymbolTable() = default;
    SymbolTable() = default;

    /// @return `nullptr` if not found.
    const SymbolEntry *lookup(const std::string &p_name) const;

    SymbolEntry *addSymbol(const std::string &p_name,
                           const SymbolEntry::KindEnum p_kind,
                           const size_t p_level, const PType *const p_p_type,
                           const Constant *const p_constant,
                           const int p_offset);
    SymbolEntry *addSymbol(const std::string &p_name,
                           const SymbolEntry::KindEnum p_kind,
                           const size_t p_level, const PType *const p_p_type,
                           const FunctionNode::DeclNodes *const p_parameters,
                           const int p_offset);
    void dump() const;

    const std::vector<std::unique_ptr<SymbolEntry>> &getEntries() const {
        return m_entries;
    }
};

class SymbolManager {
  public:
    using Table = std::unique_ptr<SymbolTable>;

  private:
    std::vector<Table> m_tables;
    int m_global_offset = -12;
    int m_label_index = 1;

    const bool m_opt_dmp;

  public:
    ~SymbolManager() = default;
    SymbolManager(const bool p_opt_dmp) : m_opt_dmp(p_opt_dmp) {}

    // initial construction
    void pushScope();
    Table popScope();
    /// @brief Pushes the given table as the new scope.
    void pushScope(Table p_table);

    /// @tparam AttributeType `Constant` or `FunctionNode::DeclNodes`
    /// @return The entry of the added symbol; `nullptr` if already exists in
    /// the current scope.
    /// @note Knows nothing about special shadowing rules, such as the shadowing
    /// of loop variables. The caller should handle them.
    template <typename AttributeType>
    SymbolEntry *addSymbol(const std::string &p_name,
                           const SymbolEntry::KindEnum p_kind,
                           const PType *const p_p_type,
                           const AttributeType *const p_attribute);

    /// @brief Looks up the symbol from the current table to the global table.
    /// @param p_name
    /// @return `nullptr` if not found.
    const SymbolEntry *lookup(const std::string &p_name) const;

    /// @return `nullptr` if no scope is pushed.
    const SymbolTable *getCurrentTable() const;

    /// @note Overflows if no scope is pushed.
    size_t getCurrentLevel() const;

    int getGlobalOffset() const { return m_global_offset; }

    void setGlobalOffset(int p_offset) { m_global_offset = p_offset; }

    int getNewLabel() { return m_label_index++; }
};

#endif
