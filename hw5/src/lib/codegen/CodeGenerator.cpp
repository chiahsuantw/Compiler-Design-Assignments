#include "codegen/CodeGenerator.hpp"

#include <algorithm>
#include <cassert>
#include <cstdarg>
#include <cstdio>
#include <memory>
#include <string>
#include <unordered_map>
#include <utility>

#include "AST/CompoundStatement.hpp"
#include "AST/for.hpp"
#include "AST/function.hpp"
#include "AST/program.hpp"
#include "sema/SemanticAnalyzer.hpp"
#include "sema/SymbolTable.hpp"
#include "visitor/AstNodeInclude.hpp"

CodeGenerator::CodeGenerator(
    const std::string &source_file_name, const std::string &save_path,
    std::unordered_map<SemanticAnalyzer::AstNodeAddr, SymbolManager::Table>
        &&p_symbol_table_of_scoping_nodes)
    : m_symbol_manager(false /* no dump */),
      m_source_file_path(source_file_name),
      m_symbol_table_of_scoping_nodes(
          std::move(p_symbol_table_of_scoping_nodes)) {
    // FIXME: assume that the source file is always xxxx.p
    const auto &real_path = save_path.empty() ? std::string{"."} : save_path;
    auto slash_pos = source_file_name.rfind('/');
    auto dot_pos = source_file_name.rfind('.');

    if (slash_pos != std::string::npos) {
        ++slash_pos;
    } else {
        slash_pos = 0;
    }
    auto output_file_path{
        real_path + "/" +
        source_file_name.substr(slash_pos, dot_pos - slash_pos) + ".S"};
    m_output_file.reset(fopen(output_file_path.c_str(), "w"));
    assert(m_output_file.get() && "Failed to open output file");
}

static void dumpInstructions(FILE *p_out_file, const char *format, ...) {
    va_list args;
    va_start(args, format);
    vfprintf(p_out_file, format, args);
    va_end(args);
}

void CodeGenerator::visit(ProgramNode &p_program) {
    // Generate RISC-V instructions for program header
    dumpInstructions(m_output_file.get(),
                     "    .file \"%s\"\n"
                     "    .option nopic\n",
                     m_source_file_path.c_str());

    // Reconstruct the scope for looking up the symbol entry.
    // Hint: Use m_symbol_manager->lookup(symbol_name) to get the symbol entry.
    m_symbol_manager.pushScope(
        std::move(m_symbol_table_of_scoping_nodes.at(&p_program)));

    auto visit_ast_node = [&](auto &ast_node) { ast_node->accept(*this); };
    for_each(p_program.getDeclNodes().begin(), p_program.getDeclNodes().end(),
             visit_ast_node);
    for_each(p_program.getFuncNodes().begin(), p_program.getFuncNodes().end(),
             visit_ast_node);

    dumpInstructions(m_output_file.get(),
                     "\n.section    .text\n"
                     "    .align 2\n"
                     "    .globl main\n"
                     "    .type main, @function\n"
                     "main:\n"
                     "    addi sp, sp, -128\n"
                     "    sw ra, 124(sp)\n"
                     "    sw s0, 120(sp)\n"
                     "    addi s0, sp, 128 # end of main prologue\n");

    const_cast<CompoundStatementNode &>(p_program.getBody()).accept(*this);

    dumpInstructions(m_output_file.get(),
                     "    lw ra, 124(sp)   # start of main epilogue\n"
                     "    lw s0, 120(sp)\n"
                     "    addi sp, sp, 128\n"
                     "    jr ra\n"
                     "    .size main, .-main\n");

    m_symbol_manager.popScope();
}

void CodeGenerator::visit(DeclNode &p_decl) { p_decl.visitChildNodes(*this); }

void CodeGenerator::visit(VariableNode &p_variable) {
    const SymbolEntry *sym = m_symbol_manager.lookup(p_variable.getName());
    if (sym->getLevel() == 0) {  // Global variable
        if (sym->getKind() == SymbolEntry::KindEnum::kVariableKind)
            dumpInstructions(m_output_file.get(), ".comm %s, 4, 4\n",
                             p_variable.getName().c_str());
        else if (sym->getKind() == SymbolEntry::KindEnum::kConstantKind) {
            constexpr const char *const assembly =
                ".section    .rodata\n"
                "    .align 2\n"
                "    .globl %s\n"
                "    .type %s, @object\n"
                "%s:\n"
                "    .word %d\n";
            dumpInstructions(
                m_output_file.get(), assembly, p_variable.getName().c_str(),
                p_variable.getName().c_str(), p_variable.getName().c_str(),
                p_variable.getConstantPtr()->integer());
        }
    } else if (sym->getLevel() > 0 &&
               p_variable.getConstantPtr()) {  // Local variable
        dumpInstructions(
            m_output_file.get(),
            "    addi t0, s0, %d\n"
            "    addi sp, sp, -4\n"
            "    sw t0, 0(sp)     # push the address to the stack\n",
            sym->getOffset());
        p_variable.visitChildNodes(*this);
        dumpInstructions(
            m_output_file.get(),
            "    lw t0, 0(sp)     # pop the value from the stack\n"
            "    addi sp, sp, 4\n"
            "    lw t1, 0(sp)     # pop the adddress from the stack\n"
            "    addi sp, sp, 4\n"
            "    sw t0, 0(t1)     # %s = expr\n",
            p_variable.getName().c_str());
    }
}

void CodeGenerator::visit(ConstantValueNode &p_constant_value) {
    dumpInstructions(m_output_file.get(),
                     "    li t0, %d\n"
                     "    addi sp, sp, -4\n"
                     "    sw t0, 0(sp)     # push the value to the stack\n",
                     p_constant_value.getConstantPtr()->integer());
}

void CodeGenerator::visit(FunctionNode &p_function) {
    // Reconstruct the scope for looking up the symbol entry.
    m_symbol_manager.pushScope(
        std::move(m_symbol_table_of_scoping_nodes.at(&p_function)));

    dumpInstructions(m_output_file.get(),
                     "\n.section    .text\n"
                     "    .align 2\n"
                     "    .globl %s\n"
                     "    .type %s, @function\n"
                     "%s:\n"
                     "    addi sp, sp, -128\n"
                     "    sw ra, 124(sp)\n"
                     "    sw s0, 120(sp)\n"
                     "    addi s0, sp, 128 # end of function prologue\n",
                     p_function.getName().c_str(), p_function.getName().c_str(),
                     p_function.getName().c_str());

    for (int args_count = 0;
         auto &entry : m_symbol_manager.getCurrentTable()->getEntries()) {
        if (entry->getKind() == SymbolEntry::KindEnum::kParameterKind) {
            if (args_count <= 7) {  // From register a0-a7
                dumpInstructions(m_output_file.get(), "    sw a%d, %d(s0)\n",
                                 args_count, entry->getOffset());
            } else {  // From spilled registers
                dumpInstructions(m_output_file.get(), "    sw t%d, %d(s0)\n",
                                 args_count - 8, entry->getOffset());
            }
            args_count++;
        }
    }

    p_function.visitBodyChildNodes(*this);

    dumpInstructions(m_output_file.get(),
                     "    lw ra, 124(sp)   # start of function epilogue\n"
                     "    lw s0, 120(sp)\n"
                     "    addi sp, sp, 128\n"
                     "    jr ra\n"
                     "    .size %s, .-%s\n\n",
                     p_function.getName().c_str(),
                     p_function.getName().c_str());

    // Remove the entries in the hash table
    m_symbol_manager.popScope();
}

void CodeGenerator::visit(CompoundStatementNode &p_compound_statement) {
    // Reconstruct the scope for looking up the symbol entry.
    m_symbol_manager.pushScope(
        std::move(m_symbol_table_of_scoping_nodes.at(&p_compound_statement)));

    p_compound_statement.visitChildNodes(*this);

    m_symbol_manager.popScope();
}

void CodeGenerator::visit(PrintNode &p_print) {
    p_print.visitChildNodes(*this);
    dumpInstructions(
        m_output_file.get(),
        "    lw a0, 0(sp)     # pop the value from the stack to a0\n"
        "    addi sp, sp, 4\n"
        "    jal ra, printInt # call function `printInt`\n");
}

void CodeGenerator::visit(BinaryOperatorNode &p_bin_op) {
    p_bin_op.visitChildNodes(*this);
    dumpInstructions(m_output_file.get(),
                     "    lw t0, 0(sp)     # pop the value from the stack\n"
                     "    addi sp, sp, 4\n"
                     "    lw t1, 0(sp)     # pop the adddress from the stack\n"
                     "    addi sp, sp, 4\n");
    switch (p_bin_op.getOp()) {
        case Operator::kPlusOp:
            dumpInstructions(m_output_file.get(), "    add t0, t1, t0\n");
            break;
        case Operator::kMinusOp:
            dumpInstructions(m_output_file.get(), "    sub t0, t1, t0\n");
            break;
        case Operator::kMultiplyOp:
            dumpInstructions(m_output_file.get(), "    mul t0, t1, t0\n");
            break;
        case Operator::kDivideOp:
            dumpInstructions(m_output_file.get(), "    div t0, t1, t0\n");
            break;
        case Operator::kModOp:
            dumpInstructions(m_output_file.get(), "    rem t0, t1, t0\n");
            break;
        // TODO: AND OR NOT
        case Operator::kEqualOp:
            dumpInstructions(m_output_file.get(), "    bne t1, t0, ");
            break;
        case Operator::kNotEqualOp:
            dumpInstructions(m_output_file.get(), "    beq t1, t0, ");
            break;
        case Operator::kGreaterOp:
            dumpInstructions(m_output_file.get(), "    ble t1, t0, ");
            break;
        case Operator::kGreaterOrEqualOp:
            dumpInstructions(m_output_file.get(), "    blt t1, t0, ");
            break;
        case Operator::kLessOp:
            dumpInstructions(m_output_file.get(), "    bge t1, t0, ");
            break;
        case Operator::kLessOrEqualOp: {
            dumpInstructions(m_output_file.get(), "    bgt t1, t0, ");
            break;
        }
        default:
            break;
    }
    if (p_bin_op.getOp() == Operator::kPlusOp ||
        p_bin_op.getOp() == Operator::kMinusOp ||
        p_bin_op.getOp() == Operator::kMultiplyOp ||
        p_bin_op.getOp() == Operator::kDivideOp ||
        p_bin_op.getOp() == Operator::kModOp) {
        dumpInstructions(
            m_output_file.get(),
            "    addi sp, sp, -4\n"
            "    sw t0, 0(sp)     # push the result to the stack\n");
    }
}

void CodeGenerator::visit(UnaryOperatorNode &p_un_op) {
    p_un_op.visitChildNodes(*this);
    dumpInstructions(m_output_file.get(),
                     "    lw t0, 0(sp)     # pop the value from the stack\n"
                     "    addi sp, sp, 4\n"
                     "    neg t0, t0\n"
                     "    addi sp, sp, -4\n"
                     "    sw t0, 0(sp)     # push the result to the stack\n");
}

void CodeGenerator::visit(FunctionInvocationNode &p_func_invocation) {
    p_func_invocation.visitChildNodes(*this);
    int args_count = p_func_invocation.getArguments().size();
    for (int i = args_count - 1; i >= 0; i--) {
        if (i > 7)
            dumpInstructions(m_output_file.get(), "    lw t%d, 0(sp)\n", i - 8);
        else
            dumpInstructions(m_output_file.get(), "    lw a%d, 0(sp)\n", i);
        dumpInstructions(m_output_file.get(), "    addi sp, sp, 4\n");
    }
    dumpInstructions(m_output_file.get(), "    jal ra, %s\n",
                     p_func_invocation.getName().c_str());
    if (p_func_invocation.getInferredType()->getPrimitiveType() !=
        PType::PrimitiveTypeEnum::kVoidType) {
        dumpInstructions(m_output_file.get(),
                         "    mv t0, a0\n"
                         "    addi sp, sp, -4\n"
                         "    sw t0, 0(sp)\n");
    }
}

void CodeGenerator::visit(VariableReferenceNode &p_variable_ref) {
    const SymbolEntry *sym = m_symbol_manager.lookup(p_variable_ref.getName());
    if (isAssignment) {
        if (sym->getLevel() == 0) {
            dumpInstructions(
                m_output_file.get(),
                "    la t0, %s\n"
                "    addi sp, sp, -4\n"
                "    sw t0, 0(sp)     # push the address to the stack\n",
                p_variable_ref.getName().c_str());
        } else {
            dumpInstructions(
                m_output_file.get(),
                "    addi t0, s0, %d\n"
                "    addi sp, sp, -4\n"
                "    sw t0, 0(sp)     # push the address to the stack\n",
                sym->getOffset());
        }
    } else {
        if (sym->getLevel() == 0) {
            dumpInstructions(
                m_output_file.get(),
                "    la t0, %s\n"
                "    lw t1, 0(t0)     # load the value of %s\n"
                "    mv t0, t1\n"
                "    addi sp, sp, -4\n"
                "    sw t0, 0(sp)     # push the value to the stack\n",
                p_variable_ref.getName().c_str(),
                p_variable_ref.getName().c_str());
        } else {
            dumpInstructions(
                m_output_file.get(),
                "    lw t0, %d(s0)   # load the value of %s\n"
                "    addi sp, sp, -4\n"
                "    sw t0, 0(sp)     # push the value to the stack\n",
                sym->getOffset(), p_variable_ref.getName().c_str());
        }
    }
}

void CodeGenerator::visit(AssignmentNode &p_assignment) {
    setIsAssignment(true);
    const_cast<VariableReferenceNode &>(p_assignment.getLvalue()).accept(*this);
    setIsAssignment(false);
    const_cast<ExpressionNode &>(p_assignment.getExpr()).accept(*this);
    dumpInstructions(m_output_file.get(),
                     "    lw t0, 0(sp)     # pop the value from the stack\n"
                     "    addi sp, sp, 4\n"
                     "    lw t1, 0(sp)     # pop the adddress from the stack\n"
                     "    addi sp, sp, 4\n"
                     "    sw t0, 0(t1)     # %s = expr\n",
                     p_assignment.getLvalue().getName().c_str());
}

void CodeGenerator::visit(ReadNode &p_read) {
    setIsAssignment(true);
    p_read.visitChildNodes(*this);
    setIsAssignment(false);
    dumpInstructions(m_output_file.get(),
                     "    jal ra, readInt  # call function `readInt`\n"
                     "    lw t0, 0(sp)     # pop the address from the stack\n"
                     "    addi sp, sp, 4\n"
                     "    sw a0, 0(t0)     # save the return value\n");
}

void CodeGenerator::visit(IfNode &p_if) {
    int l1 = m_symbol_manager.getNewLabel();
    int l2 = m_symbol_manager.getNewLabel();
    int l3 = m_symbol_manager.getNewLabel();

    const_cast<ExpressionNode &>(p_if.getCondition()).accept(*this);
    dumpInstructions(m_output_file.get(), "L%d\nL%d:\n", l2, l1);
    const_cast<CompoundStatementNode &>(p_if.getBody()).accept(*this);
    dumpInstructions(m_output_file.get(), "    j L%d\nL%d:\n", l3, l2);
    p_if.visitElseBodyChildNodes(*this);
    dumpInstructions(m_output_file.get(), "L%d:\n", l3);
}

void CodeGenerator::visit(WhileNode &p_while) {
    int l1 = m_symbol_manager.getNewLabel();
    int l2 = m_symbol_manager.getNewLabel();
    int l3 = m_symbol_manager.getNewLabel();

    dumpInstructions(m_output_file.get(), "L%d:\n", l1);
    const_cast<ExpressionNode &>(p_while.getCondition()).accept(*this);
    dumpInstructions(m_output_file.get(), "L%d\nL%d:\n", l3, l2);
    const_cast<CompoundStatementNode &>(p_while.getBody()).accept(*this);
    dumpInstructions(m_output_file.get(), "    j L%d\nL%d:\n", l1, l3);
}

void CodeGenerator::visit(ForNode &p_for) {
    // Reconstruct the scope for looking up the symbol entry.
    m_symbol_manager.pushScope(
        std::move(m_symbol_table_of_scoping_nodes.at(&p_for)));

    int l1 = m_symbol_manager.getNewLabel();
    int l2 = m_symbol_manager.getNewLabel();
    int l3 = m_symbol_manager.getNewLabel();

    const_cast<DeclNode &>(p_for.getLoopVarDecl()).accept(*this);

    setIsAssignment(true);
    const_cast<AssignmentNode &>(p_for.getInitStmt()).accept(*this);
    setIsAssignment(false);

    const SymbolEntry *sym =
        m_symbol_manager.lookup(p_for.getInitStmt().getLvalue().getName());

    dumpInstructions(m_output_file.get(),
                     "L%d:\n"
                     "    lw t0, %d(s0)\n"
                     "    addi sp, sp, -4\n"
                     "    sw t0, 0(sp)     # push the value to the stack\n",
                     l1, sym->getOffset());
    const_cast<ExpressionNode &>(p_for.getEndCondition()).accept(*this);
    dumpInstructions(m_output_file.get(),
                     "    lw t0, 0(sp)     # pop the value from the stack\n"
                     "    addi sp, sp, 4\n"
                     "    lw t1, 0(sp)     # pop the value from the stack\n"
                     "    addi sp, sp, 4\n"
                     "    bge t1, t0, L%d\nL%d:\n",
                     l3, l2);
    const_cast<CompoundStatementNode &>(p_for.getBody()).accept(*this);
    dumpInstructions(m_output_file.get(),
                     "    addi t0, s0, %d\n"
                     "    addi sp, sp, -4\n"
                     "    sw t0, 0(sp)\n"
                     "    lw t0, %d(s0)\n"
                     "    addi sp, sp, -4\n"
                     "    sw t0, 0(sp)\n"
                     "    li t0, 1\n"
                     "    addi sp, sp, -4\n"
                     "    sw t0, 0(sp)\n"
                     "    lw t0, 0(sp)\n"
                     "    addi sp, sp, 4\n"
                     "    lw t1, 0(sp)\n"
                     "    addi sp, sp, 4\n"
                     "    add t0, t1, t0\n"
                     "    addi sp, sp, -4\n"
                     "    sw t0, 0(sp)\n"
                     "    lw t0, 0(sp)\n"
                     "    addi sp, sp, 4\n"
                     "    lw t1, 0(sp)\n"
                     "    addi sp, sp, 4\n"
                     "    sw t0, 0(t1)\n"
                     "    j L%d\n"
                     "L%d:\n",
                     sym->getOffset(), sym->getOffset(), l1, l3);

    // Remove the entries in the hash table
    m_symbol_manager.popScope();
}

void CodeGenerator::visit(ReturnNode &p_return) {
    p_return.visitChildNodes(*this);
    dumpInstructions(m_output_file.get(),
                     "    lw t0, 0(sp)     # pop the value from the stack\n"
                     "    addi sp, sp, 4\n"
                     "    mv a0, t0        # load the value to ret register\n");
}
