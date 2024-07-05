#ifndef AST_P_TYPE_H
#define AST_P_TYPE_H

#include <memory>
#include <string>
#include <vector>

class PType;

using PTypeSharedPtr = std::shared_ptr<PType>;

class PType {
  public:
    enum class PrimitiveTypeEnum : uint8_t {
        kVoidType,
        kIntegerType,
        kRealType,
        kBoolType,
        kStringType
    };

  public:
    PrimitiveTypeEnum m_type;
    std::vector<uint64_t> m_dimensions;
    mutable std::string m_type_string;
    mutable bool m_type_string_is_valid = false;

  public:
    ~PType() = default;
    PType(const PrimitiveTypeEnum type) : m_type(type) {}

    void setDimensions(std::vector<uint64_t> &p_dims) {
        m_dimensions = std::move(p_dims);
    }

    PrimitiveTypeEnum getPrimitiveType() const { return m_type; }
    const char *getPTypeCString() const;
    
    PType *getStructElementType(const std::size_t nth) const {
      if (nth > m_dimensions.size())
        return nullptr;
      auto *type_ptr = new PType(m_type);
      std::vector<uint64_t> dims;
      for (std::size_t i = nth; i < m_dimensions.size(); ++i)
        dims.emplace_back(m_dimensions[i]);
      type_ptr->setDimensions(dims);
      return type_ptr;
    }

    bool compare(const PType *p_type) const;
};

#endif
