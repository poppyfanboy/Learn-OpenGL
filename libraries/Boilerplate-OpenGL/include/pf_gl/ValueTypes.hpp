/**
 * Contains enums for value types and also following functions defined:
 * - for getting related metadata
 * - for converting between OpenGL data types enums and the ones defined here
 */

// FIXME(poppyfanboy) This whole header is a complete disaster, not sure what to do with it.

#ifndef VALUE_TYPES_HPP
#define VALUE_TYPES_HPP

#include <variant>
#include <string>
#include <limits>

#include <glad/glad.h>
#include <glm/glm.hpp>

namespace pf::gl::types
{

// * Value types definitions *

// Floats
using Float = GLfloat;
using FVec2 = glm::vec2;
using FVec3 = glm::vec3;
using FVec4 = glm::vec4;

// Doubles
using Double = GLdouble;
using DVec2 = glm::dvec2;
using DVec3 = glm::dvec3;
using DVec4 = glm::dvec4;

// Float matrices
using FMat2 = glm::mat2;
using FMat3 = glm::mat3;
using FMat4 = glm::mat4;

// Unsigned ints
using UInt = GLuint;

// Signed ints
using Int = GLint;
using IntVec2 = glm::ivec2;

// Other types
using Size = GLsizei;
using BinarySize = GLsizeiptr;
using Bool = GLboolean;
using Byte = GLbyte;


// * Default values *

template <typename T>
inline T constexpr DEFAULT_VALUE;

template <>
inline Float constexpr DEFAULT_VALUE<Float> = Float(0.0F);

template <>
inline FVec2 constexpr DEFAULT_VALUE<FVec2> = FVec2(Float(0.0F), Float(0.0F));

template <>
inline FVec3 constexpr DEFAULT_VALUE<FVec3> = FVec3(Float(0.0F), Float(0.0F), Float(0.0F));

template <>
inline FVec4 constexpr DEFAULT_VALUE<FVec4> =
    FVec4(Float(0.0F), Float(0.0F), Float(0.0F), Float(0.0F));

template <>
inline Double constexpr DEFAULT_VALUE<Double> = Double(0.0);

template <>
inline DVec2 constexpr DEFAULT_VALUE<DVec2> = DVec2(Double(0.0), Double(0.0));

template <>
inline DVec3 constexpr DEFAULT_VALUE<DVec3> = DVec3(Double(0.0), Double(0.0), Double(0.0));

template <>
inline DVec4 constexpr DEFAULT_VALUE<DVec4> =
    DVec4(Double(0.0), Double(0.0), Double(0.0), Double(0.0));

// identity matrix in a row major order
template <>
inline FMat2 constexpr DEFAULT_VALUE<FMat2> = FMat2(Float(1.0F),
                                                    Float(0.0F),

                                                    Float(0.0F),
                                                    Float(1.0F));

// identity matrix in a row major order
template <>
inline FMat3 constexpr DEFAULT_VALUE<FMat3> = FMat3(Float(1.0F),
                                                    Float(0.0F),
                                                    Float(0.0F),

                                                    Float(0.0F),
                                                    Float(1.0F),
                                                    Float(0.0F),

                                                    Float(0.0F),
                                                    Float(0.0F),
                                                    Float(1.0F));

// identity matrix in a row major order
template <>
inline FMat4 constexpr DEFAULT_VALUE<FMat4> = FMat4(Float(1.0F),
                                                    Float(0.0F),
                                                    Float(0.0F),
                                                    Float(0.0F),

                                                    Float(0.0F),
                                                    Float(1.0F),
                                                    Float(0.0F),
                                                    Float(0.0F),

                                                    Float(0.0F),
                                                    Float(0.0F),
                                                    Float(1.0F),
                                                    Float(0.0F),

                                                    Float(0.0F),
                                                    Float(0.0F),
                                                    Float(0.0F),
                                                    Float(1.0F));

template <>
inline UInt constexpr DEFAULT_VALUE<UInt> = UInt(0);

template <>
inline Int constexpr DEFAULT_VALUE<Int> = Int(0);

template <>
inline IntVec2 constexpr DEFAULT_VALUE<IntVec2> = IntVec2(Int(0), Int(0));

// template <>
// constexpr inline Size DEFAULT_VALUE<Size> = Size(0);

template <>
inline BinarySize constexpr DEFAULT_VALUE<BinarySize> = BinarySize(0);

template <>
inline Bool constexpr DEFAULT_VALUE<Bool> = Bool(false);

template <>
inline Byte constexpr DEFAULT_VALUE<Byte> = Byte(0);


// * "Error" values *

/**
 * (They are just meant to be used as a convention, there's nothing wrong with these values.)
 * ! DO NOT compare anything to these values with a regular equality operator, it might give wrong
 * results (e.g. comparing anything to a NaN will return false). You should use an `isErrorValue`
 * function.
 */
template <typename T>
inline T constexpr ERROR_VALUE;

template <>
inline Float constexpr ERROR_VALUE<Float> = std::numeric_limits<Float>::quiet_NaN();

template <>
inline FVec2 constexpr ERROR_VALUE<FVec2> = FVec2(ERROR_VALUE<Float>, ERROR_VALUE<Float>);

template <>
inline FVec3 constexpr ERROR_VALUE<FVec3> = FVec3(ERROR_VALUE<Float>,
                                                  ERROR_VALUE<Float>,
                                                  ERROR_VALUE<Float>);

template <>
inline FVec4 constexpr ERROR_VALUE<FVec4> =
    FVec4(ERROR_VALUE<Float>, ERROR_VALUE<Float>, ERROR_VALUE<Float>, ERROR_VALUE<Float>);

// ? It's not necessary to set every single element of the matrix to NaN, just for a good measure
template <>
inline FMat2 constexpr ERROR_VALUE<FMat2> = FMat2(ERROR_VALUE<Float>,
                                                  ERROR_VALUE<Float>,

                                                  ERROR_VALUE<Float>,
                                                  ERROR_VALUE<Float>);

template <>
inline FMat3 constexpr ERROR_VALUE<FMat3> = FMat3(ERROR_VALUE<Float>,
                                                  ERROR_VALUE<Float>,
                                                  ERROR_VALUE<Float>,

                                                  ERROR_VALUE<Float>,
                                                  ERROR_VALUE<Float>,
                                                  ERROR_VALUE<Float>,

                                                  ERROR_VALUE<Float>,
                                                  ERROR_VALUE<Float>,
                                                  ERROR_VALUE<Float>);

template <>
inline FMat4 constexpr ERROR_VALUE<FMat4> = FMat4(ERROR_VALUE<Float>,
                                                  ERROR_VALUE<Float>,
                                                  ERROR_VALUE<Float>,
                                                  ERROR_VALUE<Float>,

                                                  ERROR_VALUE<Float>,
                                                  ERROR_VALUE<Float>,
                                                  ERROR_VALUE<Float>,
                                                  ERROR_VALUE<Float>,

                                                  ERROR_VALUE<Float>,
                                                  ERROR_VALUE<Float>,
                                                  ERROR_VALUE<Float>,
                                                  ERROR_VALUE<Float>,

                                                  ERROR_VALUE<Float>,
                                                  ERROR_VALUE<Float>,
                                                  ERROR_VALUE<Float>,
                                                  ERROR_VALUE<Float>);

template <>
inline Double constexpr ERROR_VALUE<Double> = std::numeric_limits<Double>::quiet_NaN();

template <>
inline DVec2 constexpr ERROR_VALUE<DVec2> = DVec2(ERROR_VALUE<Double>, ERROR_VALUE<Double>);

template <>
inline DVec3 constexpr ERROR_VALUE<DVec3> = FVec3(ERROR_VALUE<Double>,
                                                  ERROR_VALUE<Double>,
                                                  ERROR_VALUE<Double>);

template <>
inline DVec4 constexpr ERROR_VALUE<DVec4> =
    FVec4(ERROR_VALUE<Double>, ERROR_VALUE<Double>, ERROR_VALUE<Double>, ERROR_VALUE<Double>);


// * Utility types *

// Size is commented out because std::variant does not like it, when there are repeated types.
// (Currently Size is an alias for GLsizei which is in turn a typedef for int.)
using ValueTypeVariant = std::variant<Float,
                                      FVec2,
                                      FVec3,
                                      FVec4,
                                      Double,
                                      DVec2,
                                      DVec3,
                                      DVec4,
                                      FMat2,
                                      FMat3,
                                      FMat4,
                                      UInt,
                                      Int,
                                      IntVec2,
                                      // Size,
                                      BinarySize,
                                      Bool,
                                      Byte>;

enum ValueType
{
    FLOAT,
    FLOAT_VECTOR_2,
    FLOAT_VECTOR_3,
    FLOAT_VECTOR_4,
    DOUBLE,
    DOUBLE_VECTOR_2,
    DOUBLE_VECTOR_3,
    DOUBLE_VECTOR_4,
    FLOAT_MATRIX_2,
    FLOAT_MATRIX_3,
    FLOAT_MATRIX_4,
    UNSIGNED_INT,
    INT,
    INT_VECTOR_2,
    SIZE,
    BINARY_SIZE,
    BOOL,
    BYTE,
};


// * Utility functions *

/**
 * For a given compound type (like FVec3 or FMat4) returns a pointer to the first
 * element (in case the scalar values are stored continuously). In case a scalar value
 * is passed, returns a pointer to this scalar value.
 */
template <typename CompoundType, typename ScalarType>
ScalarType *dataPointer(CompoundType &value);

template <typename ValueType>
bool isErrorValue(ValueType const &value);

/**
 * For the given value type returns its corresponding `GLenum`. In case the value type
 * is compound, returns the scalar type of this compound type.
 * @returns `GL_FALSE` in case there is no corresponding `GLenum` for the given type.
 */
GLenum openglScalar(ValueType valueType);

/**
 * For the compound type returns how many scalar values it contains. For the scalar
 * types returns just a value of 1.
 */
Size scalarCount(ValueType valueType);

BinarySize sizeInBytes(ValueType valueType);

/**
 * Provides a placeholder value for the variable of the given type. 0 for numbers and
 * vectors, identity matrices.
 */
ValueTypeVariant defaultValue(ValueType valueType);

/**
 * A human readable name of the type for logging.
 */
std::string name(ValueType valueType);


// * Templates definitions *

template <typename CompoundType, typename ScalarType>
ScalarType *dataPointer(CompoundType &value)
{
    return &value;
}

// FIXME(poppyfanboy) This helps to avoid linker errors, but I don't know what's going
// on here and why it works.

template <>
Float *dataPointer(FVec2 &vector);

template <>
Float *dataPointer(FVec3 &vector);

template <>
Float *dataPointer(FVec4 &vector);

template <>
Double *dataPointer(DVec2 &vector);

template <>
Double *dataPointer(DVec3 &vector);

template <>
Double *dataPointer(DVec4 &vector);

template <>
Float *dataPointer(FMat2 &matrix);

template <>
Float *dataPointer(FMat3 &matrix);

template <>
Float *dataPointer(FMat4 &matrix);

template <>
Int *dataPointer(IntVec2 &vector);

template <>
bool isErrorValue(Float const &value);

template <>
bool isErrorValue(FVec2 const &value);

template <>
bool isErrorValue(FVec3 const &value);

template <>
bool isErrorValue(FVec4 const &value);

template <>
bool isErrorValue(FMat2 const &value);

template <>
bool isErrorValue(FMat3 const &value);

template <>
bool isErrorValue(FMat4 const &value);

template <>
bool isErrorValue(Double const &value);

template <>
bool isErrorValue(DVec2 const &value);

template <>
bool isErrorValue(DVec3 const &value);

template <>
bool isErrorValue(DVec4 const &value);

} // namespace pf::gl::types

#endif // !VALUE_TYPES_HPP
