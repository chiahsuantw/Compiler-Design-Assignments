#ifndef SEMA_SEMA_VALIDATOR_H
#define SEMA_SEMA_VALIDATOR_H

#include "AST/BinaryOperator.hpp"
#include "AST/PType.hpp"
#include "AST/UnaryOperator.hpp"
#include "AST/operator.hpp"

#include <iostream>

class Validator {
  public:
    static bool checkOperandsArith(Operator op, PType *ltype, PType *rtype) {
      if (op == Operator::kPlusOp &&
          ltype->m_type == PType::PrimitiveTypeEnum::kStringType &&
          rtype->m_type == PType::PrimitiveTypeEnum::kStringType)
        return true;
      if ((ltype->m_type == PType::PrimitiveTypeEnum::kIntegerType ||
           ltype->m_type == PType::PrimitiveTypeEnum::kRealType) &&
          (rtype->m_type == PType::PrimitiveTypeEnum::kIntegerType ||
           rtype->m_type == PType::PrimitiveTypeEnum::kRealType))
        return true;
      return false;
    }
    static bool checkOperandsMod(PType *ltype, PType *rtype) {
      return ltype->m_type == PType::PrimitiveTypeEnum::kIntegerType &&
             rtype->m_type == PType::PrimitiveTypeEnum::kIntegerType;
    };
    static bool checkOperandsBoolean(PType *ltype, PType *rtype) {
      return ltype->m_type == PType::PrimitiveTypeEnum::kBoolType &&
             rtype->m_type == PType::PrimitiveTypeEnum::kBoolType;
    };
    static bool checkOperandsRelation(PType *ltype, PType *rtype) {
      return (ltype->m_type == PType::PrimitiveTypeEnum::kIntegerType ||
              ltype->m_type == PType::PrimitiveTypeEnum::kRealType) &&
             (rtype->m_type == PType::PrimitiveTypeEnum::kIntegerType ||
              rtype->m_type == PType::PrimitiveTypeEnum::kRealType);
    };
    static bool checkBinOperands(BinaryOperatorNode &p_bin_op) {
      auto ltype = p_bin_op.m_left_operand.get()->getType();
      auto rtype = p_bin_op.m_right_operand.get()->getType();
      if (ltype == nullptr || rtype == nullptr)
        return false;

      if (ltype->m_dimensions.size() != rtype->m_dimensions.size())
        return false;

      switch (p_bin_op.m_op) {
      case Operator::kPlusOp:
      case Operator::kMinusOp:
      case Operator::kMultiplyOp:
      case Operator::kDivideOp:
        if (checkOperandsArith(p_bin_op.m_op, ltype, rtype))
          return true;
        break;
      case Operator::kModOp:
        if (checkOperandsMod(ltype, rtype))
          return true;
        break;
      case Operator::kAndOp:
      case Operator::kOrOp:
        if (checkOperandsBoolean(ltype, rtype))
          return true;
        break;
      case Operator::kLessOp:
      case Operator::kLessOrEqualOp:
      case Operator::kEqualOp:
      case Operator::kGreaterOp:
      case Operator::kGreaterOrEqualOp:
      case Operator::kNotEqualOp:
        if (checkOperandsRelation(ltype, rtype))
          return true;
        break;
      default:
        break;
      }
      return false;
    };
    static bool checkUnOperand(UnaryOperatorNode &p_un_op) {
      auto type = p_un_op.m_operand.get()->getType();
      if (type == nullptr)
        return false;

      if (type->m_dimensions.size() > 0)
        return false;

      switch (p_un_op.m_op) {
      case Operator::kNegOp:
        return type->m_type == PType::PrimitiveTypeEnum::kIntegerType ||
               type->m_type == PType::PrimitiveTypeEnum::kRealType;
      case Operator::kNotOp:
        return type->m_type == PType::PrimitiveTypeEnum::kBoolType;
      default:
        break;
      }
      return false;
    };
};

#endif // SEMA_SEMA_VALIDATOR_H