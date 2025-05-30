#ifndef SEMA_SEMANTIC_ANALYZER_H
#define SEMA_SEMANTIC_ANALYZER_H

#include "sema/ErrorPrinter.hpp"
#include "sema/SymbolTable.hpp"
#include "visitor/AstNodeVisitor.hpp"

#include <stack>

class SemanticAnalyzer final : public AstNodeVisitor {
  private:
    enum Context { GLOBAL, FUNCTION, FOR_LOOP, LOCAL };
    ErrorPrinter error_printer{stderr};
    SymbolManager symbol_manager;
    std::stack<Context> context_stack;
    std::stack<PType *> ret_type_stack;

  public:
    bool has_error{false};

    ~SemanticAnalyzer() = default;
    SemanticAnalyzer() = default;

    void visit(ProgramNode &p_program) override;
    void visit(DeclNode &p_decl) override;
    void visit(VariableNode &p_variable) override;
    void visit(ConstantValueNode &p_constant_value) override;
    void visit(FunctionNode &p_function) override;
    void visit(CompoundStatementNode &p_compound_statement) override;
    void visit(PrintNode &p_print) override;
    void visit(BinaryOperatorNode &p_bin_op) override;
    void visit(UnaryOperatorNode &p_un_op) override;
    void visit(FunctionInvocationNode &p_func_invocation) override;
    void visit(VariableReferenceNode &p_variable_ref) override;
    void visit(AssignmentNode &p_assignment) override;
    void visit(ReadNode &p_read) override;
    void visit(IfNode &p_if) override;
    void visit(WhileNode &p_while) override;
    void visit(ForNode &p_for) override;
    void visit(ReturnNode &p_return) override;
};

#endif
