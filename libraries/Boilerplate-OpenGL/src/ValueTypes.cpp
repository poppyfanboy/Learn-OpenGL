#include <pf_gl/ValueTypes.hpp>

#include <string>
#include <stdexcept>
#include <cmath>

#include <sparsepp/spp.h>
#include <glm/glm.hpp>

namespace pf::gl::types
{

template <>
Float *dataPointer(FVec2 &vector)
{
    return &vector.x;
}

template <>
Float *dataPointer(FVec3 &vector)
{
    return &vector.x;
}

template <>
Float *dataPointer(FVec4 &vector)
{
    return &vector.x;
}

template <>
Double *dataPointer(DVec2 &vector)
{
    return &vector.x;
}

template <>
Double *dataPointer(DVec3 &vector)
{
    return &vector.x;
}

template <>
Double *dataPointer(DVec4 &vector)
{
    return &vector.x;
}

template <>
Float *dataPointer(FMat2 &matrix)
{
    return &matrix[0].x;
}

template <>
Float *dataPointer(FMat3 &matrix)
{
    return &matrix[0].x;
}

template <>
Float *dataPointer<FMat4, Float>(FMat4 &matrix)
{
    return &matrix[0].x;
}

template <>
Int *dataPointer(IntVec2 &vector)
{
    return &vector.x;
}

template <>
bool isErrorValue(Float const &value)
{
    return std::isnan(value);
}

template <>
bool isErrorValue(FVec2 const &value)
{
    return std::isnan(value.x);
}

template <>
bool isErrorValue(FVec3 const &value)
{
    return std::isnan(value.x);
}

template <>
bool isErrorValue(FVec4 const &value)
{
    return std::isnan(value.x);
}

template <>
bool isErrorValue(FMat2 const &value)
{
    return std::isnan(value[9].x);
}

template <>
bool isErrorValue(FMat3 const &value)
{
    return std::isnan(value[0].x);
}

template <>
bool isErrorValue(FMat4 const &value)
{
    return std::isnan(value[0].x);
}

template <>
bool isErrorValue(Double const &value)
{
    return std::isnan(value);
}

template <>
bool isErrorValue(DVec2 const &value)
{
    return std::isnan(value.x);
}

template <>
bool isErrorValue(DVec3 const &value)
{
    return std::isnan(value.x);
}

template <>
bool isErrorValue(DVec4 const &value)
{
    return std::isnan(value.x);
}

struct ValueTypeMeta
{
    ValueType type;
    ValueType scalarType;
    GLenum openglScalarType;
    BinarySize scalarSize;
    Size count;
    ValueTypeVariant defaultValue;
    std::string name;
};

/**
 * A corresponding GLenum does not exist for every single type. In case there is no corresponding
 * GLenum, the value of GL_FALSE is stored.
 */
spp::sparse_hash_map<ValueType, ValueTypeMeta> const VALUE_TYPE_TO_META{
    {
        FLOAT,
        {
            FLOAT,
            FLOAT,
            GL_FLOAT,
            static_cast<BinarySize>(sizeof(Float)),
            static_cast<Size>(1),
            DEFAULT_VALUE<Float>,
            "Float",
        },
    },
    {
        FLOAT_VECTOR_2,
        {
            FLOAT_VECTOR_2,
            FLOAT,
            GL_FLOAT,
            static_cast<BinarySize>(sizeof(Float)),
            static_cast<Size>(2),
            DEFAULT_VALUE<FVec2>,
            "Float Vector2",
        },
    },
    {
        FLOAT_VECTOR_3,
        {
            FLOAT_VECTOR_3,
            FLOAT,
            GL_FLOAT,
            static_cast<BinarySize>(sizeof(Float)),
            static_cast<Size>(3),
            DEFAULT_VALUE<FVec3>,
            "Float Vector3",
        },
    },
    {
        FLOAT_VECTOR_4,
        {
            FLOAT_VECTOR_4,
            FLOAT,
            GL_FLOAT,
            static_cast<BinarySize>(sizeof(Float)),
            static_cast<Size>(4),
            DEFAULT_VALUE<FVec4>,
            "Float Vector4",
        },
    },
    {
        DOUBLE,
        {
            DOUBLE,
            DOUBLE,
            GL_DOUBLE,
            static_cast<BinarySize>(sizeof(Double)),
            static_cast<Size>(1),
            DEFAULT_VALUE<Double>,
            "Double",
        },
    },
    {
        DOUBLE_VECTOR_2,
        {
            DOUBLE_VECTOR_2,
            DOUBLE,
            GL_DOUBLE,
            static_cast<BinarySize>(sizeof(Double)),
            static_cast<Size>(2),
            DEFAULT_VALUE<DVec2>,
            "Double Vector2",
        },
    },
    {
        DOUBLE_VECTOR_3,
        {
            DOUBLE_VECTOR_3,
            DOUBLE,
            GL_DOUBLE,
            static_cast<BinarySize>(sizeof(Double)),
            static_cast<Size>(3),
            DEFAULT_VALUE<DVec3>,
            "Double Vector3",
        },
    },
    {
        DOUBLE_VECTOR_4,
        {
            DOUBLE_VECTOR_4,
            DOUBLE,
            GL_DOUBLE,
            static_cast<BinarySize>(sizeof(Double)),
            static_cast<Size>(4),
            DEFAULT_VALUE<DVec4>,
            "Double Vector4",
        },
    },
    {
        FLOAT_MATRIX_2,
        {
            FLOAT_MATRIX_2,
            FLOAT,
            GL_FLOAT,
            static_cast<BinarySize>(sizeof(Float)),
            static_cast<Size>(2) * 2,
            DEFAULT_VALUE<FMat2>,
            "Float Matrix2x2",
        },
    },
    {
        FLOAT_MATRIX_3,
        {
            FLOAT_MATRIX_3,
            FLOAT,
            GL_FLOAT,
            static_cast<BinarySize>(sizeof(Float)),
            static_cast<Size>(3) * 3,
            DEFAULT_VALUE<FMat3>,
            "Float Matrix3x3",
        },
    },
    {
        FLOAT_MATRIX_4,
        {
            FLOAT_MATRIX_4,
            FLOAT,
            GL_FLOAT,
            static_cast<BinarySize>(sizeof(Float)),
            static_cast<Size>(4) * 4,
            DEFAULT_VALUE<FMat4>,
            "Float Matrix4x4",
        },
    },
    {
        UNSIGNED_INT,
        {
            UNSIGNED_INT,
            UNSIGNED_INT,
            GL_UNSIGNED_INT,
            static_cast<BinarySize>(sizeof(UInt)),
            static_cast<Size>(1),
            DEFAULT_VALUE<UInt>,
            "Unsigned Integer",
        },
    },
    {
        INT,
        {
            INT,
            INT,
            GL_INT,
            static_cast<BinarySize>(sizeof(Int)),
            static_cast<Size>(1),
            DEFAULT_VALUE<Int>,
            "Integer",
        },
    },
    {
        INT_VECTOR_2,
        {
            INT_VECTOR_2,
            INT,
            GL_INT,
            static_cast<BinarySize>(sizeof(Int)),
            static_cast<Size>(2),
            DEFAULT_VALUE<IntVec2>,
            "Integer Vector2",
        },
    },
    {
        SIZE,
        {
            SIZE,
            SIZE,
            GL_FALSE,
            static_cast<BinarySize>(sizeof(Size)),
            static_cast<Size>(1),
            DEFAULT_VALUE<Size>,
            "Size Type",
        },
    },
    {
        BINARY_SIZE,
        {
            BINARY_SIZE,
            BINARY_SIZE,
            GL_FALSE,
            static_cast<BinarySize>(sizeof(BinarySize)),
            static_cast<Size>(1),
            DEFAULT_VALUE<BinarySize>,
            "Binary Size Type",
        },
    },
    {
        BOOL,
        {
            BOOL,
            BOOL,
            GL_FALSE,
            static_cast<BinarySize>(sizeof(Bool)),
            static_cast<Size>(1),
            DEFAULT_VALUE<Bool>,
            "Boolean",
        },
    },
    {
        BYTE,
        {
            BYTE,
            BYTE,
            GL_FALSE,
            static_cast<BinarySize>(sizeof(Byte)),
            static_cast<Size>(1),
            DEFAULT_VALUE<Byte>,
            "Byte",
        },
    },
};

GLenum openglScalar(ValueType valueType)
{
    if (VALUE_TYPE_TO_META.find(valueType) == VALUE_TYPE_TO_META.end())
    {
        throw std::runtime_error("Unknown vertex attribute type.");
    }

    return VALUE_TYPE_TO_META.at(valueType).openglScalarType;
}

Size scalarCount(ValueType valueType)
{
    if (VALUE_TYPE_TO_META.find(valueType) == VALUE_TYPE_TO_META.end())
    {
        throw std::runtime_error("Unknown vertex attribute type.");
    }

    return VALUE_TYPE_TO_META.at(valueType).count;
}

BinarySize sizeInBytes(ValueType valueType)
{
    if (VALUE_TYPE_TO_META.find(valueType) == VALUE_TYPE_TO_META.end())
    {
        throw std::runtime_error("Unknown vertex attribute type.");
    }

    ValueTypeMeta meta = VALUE_TYPE_TO_META.at(valueType);
    return meta.scalarSize * meta.count;
}

ValueTypeVariant defaultValue(ValueType valueType)
{
    if (VALUE_TYPE_TO_META.find(valueType) == VALUE_TYPE_TO_META.end())
    {
        throw std::runtime_error("Unknown vertex attribute type.");
    }

    return VALUE_TYPE_TO_META.at(valueType).defaultValue;
}

std::string name(ValueType valueType)
{
    if (VALUE_TYPE_TO_META.find(valueType) == VALUE_TYPE_TO_META.end())
    {
        throw std::runtime_error("Unknown vertex attribute type.");
    }

    return VALUE_TYPE_TO_META.at(valueType).name;
}

} // namespace pf::gl::types
