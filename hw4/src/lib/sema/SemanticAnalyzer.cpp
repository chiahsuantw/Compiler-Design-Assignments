#include "sema/SemanticAnalyzer.hpp"
#include "sema/Error.hpp"
#include "sema/SemaValidator.hpp"
#include "sema/SymbolTable.hpp"
#include "visitor/AstNodeInclude.hpp"

#include <set>
#include <string>

std::set<SymbolEntry *> var_decl_err_set;

void SemanticAnalyzer::visit(ProgramNode &p_program) {
  SymbolTable *table = new SymbolTable();
  symbol_manager.pushScope(table);
  context_stack.push(Context::GLOBAL);
  ret_type_stack.push(p_program.m_ret_type.get());

  if (symbol_manager.cur_table->findSymbol(p_program.m_name)) {
    has_error = true;
    error_printer.print(SymbolRedeclarationError(p_program.getLocation(),
                                                 p_program.getNameCString()));
  } else {
    symbol_manager.cur_table->addSymbol(
        p_program.m_name, SymbolEntry::Kind::PROGRAM, symbol_manager.cur_level,
        p_program.m_ret_type.get(), static_cast<Constant *>(nullptr));
  }

  p_program.visitChildNodes(*this);

  ret_type_stack.pop();
  context_stack.pop();
  symbol_manager.popScope();
  symbol_manager.dumpSymbolTable(table);
}

void SemanticAnalyzer::visit(DeclNode &p_decl) {
  p_decl.visitChildNodes(*this);
}

void SemanticAnalyzer::visit(VariableNode &p_variable) {
  SymbolEntry::Kind kind;

  Constant *constant = nullptr;
  if (p_variable.m_constant_value_node_ptr.get())
    constant = p_variable.m_constant_value_node_ptr.get()->m_constant_ptr.get();

  if (context_stack.top() == Context::FOR_LOOP)
    kind = SymbolEntry::Kind::LOOP_VAR;
  else if (context_stack.top() == Context::FUNCTION)
    kind = SymbolEntry::Kind::PARAMETER;
  else if (p_variable.m_constant_value_node_ptr.get())
    kind = SymbolEntry::Kind::CONSTANT;
  else
    kind = SymbolEntry::Kind::VARIABLE;

  bool redecl = symbol_manager.cur_table->findSymbol(p_variable.m_name);
  std::stack<SymbolTable *> temp;
  while (!symbol_manager.tables.empty()) {
    SymbolTable *table = symbol_manager.tables.top();
    temp.push(table);
    symbol_manager.tables.pop();
    auto entry = table->findSymbol(p_variable.m_name);
    redecl |= entry && entry->kind == SymbolEntry::Kind::LOOP_VAR;
  }
  while (!temp.empty()) {
    symbol_manager.tables.push(temp.top());
    temp.pop();
  }

  if (redecl) {
    has_error = true;
    error_printer.print(SymbolRedeclarationError(p_variable.getLocation(),
                                                 p_variable.getNameCString()));
  } else {
    symbol_manager.cur_table->addSymbol(p_variable.m_name, kind,
                                        symbol_manager.cur_level,
                                        p_variable.m_type.get(), constant);
    // Check array dimension
    for (auto i : p_variable.m_type.get()->m_dimensions) {
      if (i <= 0) {
        has_error = true;
        error_printer.print(NonPositiveArrayDimensionError(
            p_variable.getLocation(), p_variable.getNameCString()));
        var_decl_err_set.insert(&symbol_manager.cur_table->entries.back());
      }
    }
  }

  p_variable.visitChildNodes(*this);
}

void SemanticAnalyzer::visit(ConstantValueNode &p_constant_value) {
  p_constant_value.setType(
      p_constant_value.m_constant_ptr->m_type.get()->getStructElementType(0));
}

void SemanticAnalyzer::visit(FunctionNode &p_function) {
  if (symbol_manager.cur_table->findSymbol(p_function.m_name)) {
    has_error = true;
    error_printer.print(SymbolRedeclarationError(p_function.getLocation(),
                                                 p_function.getNameCString()));
  } else {
    symbol_manager.cur_table->addSymbol(
        p_function.m_name, SymbolEntry::Kind::FUNCTION,
        symbol_manager.cur_level, p_function.m_ret_type.get(),
        &p_function.m_parameters);
  }

  SymbolTable *table = new SymbolTable();
  symbol_manager.pushScope(table);
  context_stack.push(Context::FUNCTION);
  ret_type_stack.push(p_function.m_ret_type.get());

  for (auto &param : p_function.m_parameters)
    param->accept(*this);

  context_stack.push(Context::LOCAL);
  if (p_function.m_body)
    p_function.m_body->visitChildNodes(*this);
  context_stack.pop();

  ret_type_stack.pop();
  context_stack.pop();
  symbol_manager.popScope();
  symbol_manager.dumpSymbolTable(table);
}

void SemanticAnalyzer::visit(CompoundStatementNode &p_compound_statement) {
  SymbolTable *table = new SymbolTable();
  symbol_manager.pushScope(table);
  context_stack.push(Context::LOCAL);

  p_compound_statement.visitChildNodes(*this);

  context_stack.pop();
  symbol_manager.popScope();
  symbol_manager.dumpSymbolTable(table);
}

void SemanticAnalyzer::visit(PrintNode &p_print) {
  p_print.visitChildNodes(*this);

  auto target = p_print.m_target.get()->getType();
  if (!target) {
    has_error = true;
    return;
  }

  if (!(target->m_dimensions.empty() &&
        target->m_type != PType::PrimitiveTypeEnum::kVoidType)) {
    has_error = true;
    error_printer.print(
        PrintOutNonScalarTypeError(p_print.m_target->getLocation()));
    return;
  }
}

void SemanticAnalyzer::visit(BinaryOperatorNode &p_bin_op) {
  p_bin_op.visitChildNodes(*this);

  if (!p_bin_op.m_left_operand.get()->getType() ||
      !p_bin_op.m_right_operand.get()->getType()) {
    has_error = true;
    return;
  }

  if (!Validator::checkBinOperands(p_bin_op)) {
    has_error = true;
    error_printer.print(
        InvalidBinaryOperandError(p_bin_op.getLocation(), p_bin_op.m_op,
                                  p_bin_op.m_left_operand.get()->getType(),
                                  p_bin_op.m_right_operand.get()->getType()));
    return;
  }

  switch (p_bin_op.m_op) {
  case Operator::kPlusOp:
  case Operator::kMinusOp:
  case Operator::kMultiplyOp:
  case Operator::kDivideOp:
    if (p_bin_op.m_left_operand.get()->getType()->m_type ==
        PType::PrimitiveTypeEnum::kStringType) {
      p_bin_op.setType(new PType(PType::PrimitiveTypeEnum::kStringType));
      return;
    }
    if (p_bin_op.m_left_operand.get()->getType()->m_type ==
            PType::PrimitiveTypeEnum::kRealType ||
        p_bin_op.m_right_operand.get()->getType()->m_type ==
            PType::PrimitiveTypeEnum::kRealType) {
      p_bin_op.setType(new PType(PType::PrimitiveTypeEnum::kRealType));
      return;
    }
  case Operator::kModOp:
    p_bin_op.setType(new PType(PType::PrimitiveTypeEnum::kIntegerType));
    return;
  case Operator::kAndOp:
  case Operator::kOrOp:
    p_bin_op.setType(new PType(PType::PrimitiveTypeEnum::kBoolType));
    return;
  case Operator::kLessOp:
  case Operator::kLessOrEqualOp:
  case Operator::kEqualOp:
  case Operator::kGreaterOp:
  case Operator::kGreaterOrEqualOp:
  case Operator::kNotEqualOp:
    p_bin_op.setType(new PType(PType::PrimitiveTypeEnum::kBoolType));
    return;
  default:
    return;
  }
}

void SemanticAnalyzer::visit(UnaryOperatorNode &p_un_op) {
  p_un_op.visitChildNodes(*this);

  if (!p_un_op.m_operand.get()->getType()) {
    has_error = true;
    return;
  }

  if (!Validator::checkUnOperand(p_un_op)) {
    has_error = true;
    error_printer.print(
        InvalidUnaryOperandError(p_un_op.getLocation(), p_un_op.m_op,
                                 p_un_op.m_operand.get()->getType()));
    return;
  }

  switch (p_un_op.m_op) {
  case Operator::kNegOp:
    p_un_op.setType(
        new PType(p_un_op.m_operand.get()->getType()->getPrimitiveType()));
    return;
  case Operator::kNotOp:
    p_un_op.setType(new PType(PType::PrimitiveTypeEnum::kBoolType));
    return;
  default:
    return;
  }
}

void SemanticAnalyzer::visit(FunctionInvocationNode &p_func_invocation) {
  p_func_invocation.visitChildNodes(*this);

  // Check undeclared
  auto symbol = symbol_manager.cur_table->findSymbol(p_func_invocation.m_name);
  std::stack<SymbolTable *> temp;
  while (!symbol && !symbol_manager.tables.empty()) {
    SymbolTable *table = symbol_manager.tables.top();
    temp.push(table);
    symbol_manager.tables.pop();
    symbol = table->findSymbol(p_func_invocation.getNameCString());
  }
  while (!temp.empty()) {
    symbol_manager.tables.push(temp.top());
    temp.pop();
  }
  if (!symbol) {
    has_error = true;
    error_printer.print(UndeclaredSymbolError(
        p_func_invocation.getLocation(), p_func_invocation.getNameCString()));
    return;
  }

  // Check non-function symbol
  if (symbol->kind != SymbolEntry::Kind::FUNCTION) {
    has_error = true;
    error_printer.print(NonFunctionSymbolError(
        p_func_invocation.getLocation(), p_func_invocation.getNameCString()));
    return;
  }

  // Check argument count == parameter count
  FunctionNode::DeclNodes &params = *symbol->params_ptr;
  FunctionInvocationNode::ExprNodes &args = p_func_invocation.m_args;
  int param_count = 0;
  for (auto &param : params)
    param_count += param->m_var_nodes.size();
  if (args.size() != (FunctionNode::DeclNodes::size_type)param_count) {
    has_error = true;
    error_printer.print(ArgumentNumberMismatchError(
        p_func_invocation.getLocation(), p_func_invocation.getNameCString()));
    return;
  }

  // Traverse arguments and check type
  FunctionInvocationNode::ExprNodes::const_iterator it = args.begin();
  for (auto &param : params) {
    auto &vars = param->m_var_nodes;
    for (auto &var : vars) {
      auto *expr_type = (*it)->getType();
      if (!expr_type) {
        has_error = true;
        return;
      }
      if (!expr_type->compare(var->m_type.get())) {
        has_error = true;
        error_printer.print(IncompatibleArgumentTypeError(
            (*it)->getLocation(), var->m_type.get(), expr_type));
        return;
      }
      it++;
    }
  }

  p_func_invocation.setType(new PType(symbol->type->getPrimitiveType()));
}

void SemanticAnalyzer::visit(VariableReferenceNode &p_variable_ref) {
  p_variable_ref.visitChildNodes(*this);

  // Check undeclared
  auto symbol = symbol_manager.cur_table->findSymbol(p_variable_ref.m_name);
  std::stack<SymbolTable *> temp;
  while (!symbol && !symbol_manager.tables.empty()) {
    SymbolTable *table = symbol_manager.tables.top();
    temp.push(table);
    symbol_manager.tables.pop();
    symbol = table->findSymbol(p_variable_ref.m_name);
  }
  while (!temp.empty()) {
    symbol_manager.tables.push(temp.top());
    temp.pop();
  }
  if (!symbol) {
    has_error = true;
    error_printer.print(UndeclaredSymbolError(p_variable_ref.getLocation(),
                                              p_variable_ref.getNameCString()));
    return;
  }

  // Check non-variable symbol
  if (symbol->kind != SymbolEntry::Kind::PARAMETER &&
      symbol->kind != SymbolEntry::Kind::VARIABLE &&
      symbol->kind != SymbolEntry::Kind::LOOP_VAR &&
      symbol->kind != SymbolEntry::Kind::CONSTANT) {
    has_error = true;
    error_printer.print(NonVariableSymbolError(
        p_variable_ref.getLocation(), p_variable_ref.getNameCString()));
    return;
  }

  if (var_decl_err_set.find(symbol) != var_decl_err_set.end())
    return;

  // Check non-integer array index
  for (auto &index : p_variable_ref.m_indices) {
    if (index.get()->getType() == nullptr)
      return;
    if (index.get()->getType()->getPrimitiveType() !=
        PType::PrimitiveTypeEnum::kIntegerType) {
      has_error = true;
      error_printer.print(NonIntegerArrayIndexError(index->getLocation()));
      return;
    }
  }

  // Check over array subscript
  if (symbol->type->m_dimensions.size() < p_variable_ref.m_indices.size()) {
    has_error = true;
    error_printer.print(OverArraySubscriptError(
        p_variable_ref.getLocation(), p_variable_ref.getNameCString()));
    return;
  }

  p_variable_ref.setType(
      symbol->type->getStructElementType(p_variable_ref.m_indices.size()));
}

void SemanticAnalyzer::visit(AssignmentNode &p_assignment) {
  p_assignment.visitChildNodes(*this);

  auto &lvalue = p_assignment.m_lvalue;
  if (!lvalue->getType()) {
    has_error = true;
    return;
  }

  if (!(lvalue->getType()->m_dimensions.empty() &&
        lvalue->getType()->m_type != PType::PrimitiveTypeEnum::kVoidType)) {
    has_error = true;
    error_printer.print(AssignToArrayTypeError(lvalue->getLocation()));
    return;
  }

  auto symbol = symbol_manager.cur_table->findSymbol(lvalue->m_name);
  std::stack<SymbolTable *> temp;
  while (!symbol && !symbol_manager.tables.empty()) {
    SymbolTable *table = symbol_manager.tables.top();
    temp.push(table);
    symbol_manager.tables.pop();
    symbol = table->findSymbol(lvalue->m_name);
  }
  while (!temp.empty()) {
    symbol_manager.tables.push(temp.top());
    temp.pop();
  }
  if (symbol->kind == SymbolEntry::Kind::CONSTANT) {
    has_error = true;
    error_printer.print(
        AssignToConstantError(lvalue->getLocation(), lvalue->m_name));
    return;
  }

  if (context_stack.top() != Context::FOR_LOOP &&
      symbol->kind == SymbolEntry::Kind::LOOP_VAR) {
    has_error = true;
    error_printer.print(AssignToLoopVarError(lvalue->getLocation()));
    return;
  }

  auto &expr = p_assignment.m_expr;
  if (!expr->getType()) {
    has_error = true;
    return;
  }

  if (!(expr->getType()->m_dimensions.empty() &&
        expr->getType()->m_type != PType::PrimitiveTypeEnum::kVoidType)) {
    has_error = true;
    error_printer.print(AssignWithArrayTypeError(expr->getLocation()));
    return;
  }

  if (!p_assignment.m_lvalue->getType()->compare(expr->getType())) {
    has_error = true;
    error_printer.print(IncompatibleAssignmentError(
        p_assignment.getLocation(), p_assignment.m_lvalue->getType(),
        expr->getType()));
    return;
  }
}

void SemanticAnalyzer::visit(ReadNode &p_read) {
  p_read.visitChildNodes(*this);

  auto target = p_read.m_target.get()->getType();
  if (!target) {
    has_error = true;
    return;
  }

  if (!(target->m_dimensions.empty() &&
        target->m_type != PType::PrimitiveTypeEnum::kVoidType)) {
    has_error = true;
    error_printer.print(
        ReadToNonScalarTypeError(p_read.m_target->getLocation()));
    return;
  }

  auto symbol = symbol_manager.cur_table->findSymbol(p_read.m_target->m_name);
  std::stack<SymbolTable *> temp;
  while (!symbol && !symbol_manager.tables.empty()) {
    SymbolTable *table = symbol_manager.tables.top();
    temp.push(table);
    symbol_manager.tables.pop();
    symbol = table->findSymbol(p_read.m_target->m_name);
  }
  while (!temp.empty()) {
    symbol_manager.tables.push(temp.top());
    temp.pop();
  }
  if (symbol->kind == SymbolEntry::Kind::CONSTANT ||
      symbol->kind == SymbolEntry::Kind::LOOP_VAR) {
    has_error = true;
    error_printer.print(
        ReadToConstantOrLoopVarError(p_read.m_target->getLocation()));
    return;
  }
}

void SemanticAnalyzer::visit(IfNode &p_if) {
  p_if.visitChildNodes(*this);

  if (!p_if.m_condition->getType())
    return;

  if (p_if.m_condition->getType()->m_type !=
      PType::PrimitiveTypeEnum::kBoolType) {
    has_error = true;
    error_printer.print(
        NonBooleanConditionError(p_if.m_condition->getLocation()));
    return;
  }
}

void SemanticAnalyzer::visit(WhileNode &p_while) {
  p_while.visitChildNodes(*this);

  if (!p_while.m_condition->getType())
    return;

  if (p_while.m_condition->getType()->m_type !=
      PType::PrimitiveTypeEnum::kBoolType) {
    has_error = true;
    error_printer.print(
        NonBooleanConditionError(p_while.m_condition->getLocation()));
    return;
  }
}

void SemanticAnalyzer::visit(ForNode &p_for) {
  SymbolTable *table = new SymbolTable();
  symbol_manager.pushScope(table);
  context_stack.push(Context::FOR_LOOP);

  p_for.visitChildNodes(*this);

  if (dynamic_cast<const ConstantValueNode *>(p_for.m_init_stmt->m_expr.get())
          ->m_constant_ptr->m_value.integer >=
      dynamic_cast<const ConstantValueNode *>(p_for.m_end_condition.get())
          ->m_constant_ptr->m_value.integer) {
    has_error = true;
    error_printer.print(NonIncrementalLoopVariableError(p_for.getLocation()));
  }

  context_stack.pop();
  symbol_manager.popScope();
  symbol_manager.dumpSymbolTable(table);
}

void SemanticAnalyzer::visit(ReturnNode &p_return) {
  p_return.visitChildNodes(*this);

  auto ret_type = ret_type_stack.top();
  if (ret_type->m_type == PType::PrimitiveTypeEnum::kVoidType) {
    has_error = true;
    error_printer.print(ReturnFromVoidError(p_return.getLocation()));
    return;
  }

  if (!p_return.m_ret_val->getType())
    return;

  if (!ret_type->compare(p_return.m_ret_val->getType())) {
    has_error = true;
    error_printer.print(
        IncompatibleReturnTypeError(p_return.m_ret_val->getLocation(), ret_type,
                                    p_return.m_ret_val->getType()));
    return;
  }
}
