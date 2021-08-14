#ifndef EULER_TRANSFORM_3D_HPP
#define EULER_TRANSFORM_3D_HPP

#include <memory>

#include <pf_gl/Transform3D.hpp>
#include <pf_gl/ValueTypes.hpp>

namespace pf::gl
{

/**
 * Internally uses Euler angles (YXZ order) for rotations.
 */
class EulerTransform3D : public Transform3D
{
public:
    /**
     * Setting euler angles with a vector has more priority than setting yaw/pitch/roll separately.
     * And setting rotation with a matrix has more priority than both of the beforementioned
     * methods.
     */
    class Builder
    {
    public:
        Builder &withShift(types::FVec3 const &shift);
        Builder &withYaw(types::Float yaw);
        Builder &withPitch(types::Float pitch);
        Builder &withRoll(types::Float roll);
        Builder &withEulerAngles(types::FVec3 const &eulerAngles);
        Builder &withScale(types::FVec3 const &scale);
        Builder &withRotationMatrix(types::FMat3 const &rotationMatrix);

        std::unique_ptr<Transform3D> build();

    private:
        types::FVec3 _shift = types::DEFAULT_VALUE<types::FVec3>;
        types::FVec3 _scale = IDENTITY_SCALE;

        // Set those initially to error values to detect, whether their values have been set or not
        types::Float _yaw = types::ERROR_VALUE<types::Float>;
        types::Float _pitch = types::ERROR_VALUE<types::Float>;
        types::Float _roll = types::ERROR_VALUE<types::Float>;

        types::FVec3 _eulerAngles = types::ERROR_VALUE<types::FVec3>;

        types::FMat3 _rotationMatrix = types::ERROR_VALUE<types::FMat3>;
    };

    static EulerTransform3D const IDENTITY;

    /**
     * Creates an identity transform.
     */
    EulerTransform3D();

    EulerTransform3D(types::FVec3 const &shift,
                     types::Float yaw,
                     types::Float pitch,
                     types::Float roll,
                     types::FVec3 const &scale);

    EulerTransform3D(types::FVec3 const &shift,
                     types::FMat3 const &rotationMatrix,
                     types::FVec3 const &scale);

    [[nodiscard]] types::FVec3 shift() const override;
    [[nodiscard]] types::FVec3 scale() const override;
    [[nodiscard]] types::FMat3 rotationMatrix() const override;
    [[nodiscard]] types::FMat4 localToWorldMatrix() const override;

    // * Combining with other transforms *

    [[nodiscard]] std::unique_ptr<Transform3D>
    rotate(types::FVec3 const &eulerAngles) const override;

    [[nodiscard]] std::unique_ptr<Transform3D>
    rotateAround(types::Float angle, types::FVec3 const &axis) const override;

    [[nodiscard]] std::unique_ptr<Transform3D> shift(types::FVec3 const &shift) const override;

    [[nodiscard]] std::unique_ptr<Transform3D> scale(types::FVec3 const &scale) const override;

    [[nodiscard]] std::unique_ptr<Transform3D>
    lookAt(types::FVec3 const &lookAtTarget) const override;

    [[nodiscard]] std::unique_ptr<Transform3D>
    combine(Transform3D const &otherTransform) const override;

    // * Applying the transform *

    [[nodiscard]] types::FVec3 transformDirection(types::FVec3 const &direction) const override;
    [[nodiscard]] types::FVec3 transformPoint(types::FVec3 const &point) const override;
    [[nodiscard]] types::FVec3 transformVector(types::FVec3 const &vector) const override;

    // * Applying transform to the XYZ coordinates *

    [[nodiscard]] types::FVec3 forwardVector() const override;
    [[nodiscard]] types::FVec3 upVector() const override;
    [[nodiscard]] types::FVec3 rightVector() const override;

private:
    types::FMat3 _rotationMatrix;
    types::FVec3 _shift;
    types::FVec3 _scale;
};

} // namespace pf::gl

#endif // !EULER_TRANSFORM_3D_HPP
