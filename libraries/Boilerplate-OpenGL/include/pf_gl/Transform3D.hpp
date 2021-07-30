#ifndef TRANSFORM_3D_HPP
#define TRANSFORM_3D_HPP

#include <memory>

#include <pf_gl/ValueTypes.hpp>

namespace pf::gl
{

class Transform3D
{
public:
    static inline types::FVec3 constexpr X_AXIS =
        types::FVec3(types::Float(1.0F), types::Float(0.0F), types::Float(0.0F));

    static inline types::FVec3 constexpr Y_AXIS =
        types::FVec3(types::Float(0.0F), types::Float(1.0F), types::Float(0.0F));

    static inline types::FVec3 constexpr Z_AXIS =
        types::FVec3(types::Float(0.0F), types::Float(0.0F), types::Float(1.0F));

    static inline types::FVec3 constexpr IDENTITY_SCALE =
        types::FVec3(types::Float(1.0F), types::Float(1.0F), types::Float(1.0F));

    virtual ~Transform3D() = default;

    [[nodiscard]] virtual types::FVec3 shift() const = 0;
    [[nodiscard]] virtual types::FVec3 scale() const = 0;
    [[nodiscard]] virtual types::FMat3 rotationMatrix() const = 0;
    [[nodiscard]] virtual types::FMat4 localToWorldMatrix() const = 0;

    // * Combining with other transforms *

    [[nodiscard]] virtual std::unique_ptr<Transform3D>
    rotate(types::FVec3 const &eulerAngles) const = 0;

    [[nodiscard]] virtual std::unique_ptr<Transform3D>
    rotateAround(types::Float angle, types::FVec3 const &axis) const = 0;

    [[nodiscard]] virtual std::unique_ptr<Transform3D> shift(types::FVec3 const &shift) const = 0;

    [[nodiscard]] virtual std::unique_ptr<Transform3D> scale(types::FVec3 const &scale) const = 0;

    [[nodiscard]] virtual std::unique_ptr<Transform3D>
    lookAt(types::FVec3 const &lookAtTarget) const = 0;

    [[nodiscard]] virtual std::unique_ptr<Transform3D>
    combine(Transform3D const &otherTransform) const = 0;

    // * Applying the transform *

    /**
     * Direction is not affected by scale or shift of the transform.
     */
    [[nodiscard]] virtual types::FVec3 transformDirection(types::FVec3 const &direction) const = 0;
    [[nodiscard]] virtual types::FVec3 transformPoint(types::FVec3 const &point) const = 0;
    /**
     * Vector is not affected by the shift of the transform
     */
    [[nodiscard]] virtual types::FVec3 transformVector(types::FVec3 const &vector) const = 0;

    // * Applying transform to the XYZ coordinates *

    [[nodiscard]] virtual types::FVec3 forwardVector() const = 0;
    [[nodiscard]] virtual types::FVec3 upVector() const = 0;
    [[nodiscard]] virtual types::FVec3 rightVector() const = 0;
};

} // namespace pf::gl

#endif // !TRANSFORM_3D_HPP
