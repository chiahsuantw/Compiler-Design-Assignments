#include "AST/PType.hpp"

const char *kTypeString[] = {"void", "integer", "real", "boolean", "string"};

// logical constness
const char *PType::getPTypeCString() const {
    if (!m_type_string_is_valid) {
        m_type_string += kTypeString[static_cast<size_t>(m_type)];

        if (m_dimensions.size() != 0) {
            m_type_string += " ";

            for (const auto &dim : m_dimensions) {
                m_type_string += "[" + std::to_string(dim) + "]";
            }
        }
        m_type_string_is_valid = true;
    }

    return m_type_string.c_str();
}

bool PType::compare(const PType *p_type) const {
    switch (m_type) {
    case PrimitiveTypeEnum::kIntegerType:
    case PrimitiveTypeEnum::kRealType:
        if (!(p_type->m_type == PrimitiveTypeEnum::kIntegerType) &&
            !(p_type->m_type == PrimitiveTypeEnum::kRealType))
            return false;
        break;
    case PrimitiveTypeEnum::kBoolType:
        if (!(p_type->m_type == PrimitiveTypeEnum::kBoolType))
            return false;
        break;
    case PrimitiveTypeEnum::kStringType:
        if (!(p_type->m_type == PrimitiveTypeEnum::kStringType))
            return false;
        break;
    default:
        return false;
    }
    auto &dimensions = p_type->m_dimensions;
    if (m_dimensions.size() != dimensions.size())
        return false;
    for (decltype(m_dimensions)::size_type i = 0; i < m_dimensions.size(); ++i)
        if (m_dimensions[i] != dimensions[i])
            return false;
    return true;
}
