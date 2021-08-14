#include <pf_gl/EulerTransform3D.hpp>

#include <memory>

#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>

#include <pf_gl/ValueTypes.hpp>

namespace pf::gl
{
EulerTransform3D::Builder &EulerTransform3D::Builder::withShift(types::FVec3 const &shift)
{
    _shift = shift;
    return *this;
}

EulerTransform3D::Builder &EulerTransform3D::Builder::withYaw(types::Float yaw)
{
    _yaw = yaw;
    return *this;
}

EulerTransform3D::Builder &EulerTransform3D::Builder::withPitch(types::Float pitch)
{
    _pitch = pitch;
    return *this;
}

EulerTransform3D::Builder &EulerTransform3D::Builder::withRoll(types::Float roll)
{
    _roll = roll;
    return *this;
}

EulerTransform3D::Builder &
EulerTransform3D::Builder::withEulerAngles(types::FVec3 const &eulerAngles)
{
    _eulerAngles = eulerAngles;
    return *this;
}

EulerTransform3D::Builder &EulerTransform3D::Builder::withScale(types::FVec3 const &scale)
{
    _scale = scale;
    return *this;
}

EulerTransform3D::Builder &
EulerTransform3D::Builder::withRotationMatrix(types::FMat3 const &rotationMatrix)
{
    _rotationMatrix = rotationMatrix;
    return *this;
}

std::unique_ptr<Transform3D> EulerTransform3D::Builder::build()
{
    if (!types::isErrorValue(_rotationMatrix))
    {
        return std::make_unique<EulerTransform3D>(_shift, _rotationMatrix, _scale);
    }
    if (!types::isErrorValue(_eulerAngles))
    {
        return std::make_unique<EulerTransform3D>(
            _shift, _eulerAngles.y, _eulerAngles.x, _eulerAngles.z, _scale);
    }
    if (!types::isErrorValue(_rotationMatrix))
    {
        return std::make_unique<EulerTransform3D>(_shift, _rotationMatrix, _scale);
    }
    return std::make_unique<EulerTransform3D>(_shift, types::DEFAULT_VALUE<types::FMat3>, _scale);
}

EulerTransform3D const EulerTransform3D::IDENTITY = EulerTransform3D();

EulerTransform3D::EulerTransform3D()
    : _shift(types::DEFAULT_VALUE<types::FVec3>)
    , _rotationMatrix(types::DEFAULT_VALUE<types::FMat3>)
    , _scale(IDENTITY_SCALE)
{
}

EulerTransform3D::EulerTransform3D(types::FVec3 const &shift,
                                   types::Float yaw,
                                   types::Float pitch,
                                   types::Float roll,
                                   types::FVec3 const &scale)
    : _shift(shift)
    , _scale(scale)
{
    _rotationMatrix =
        glm::rotate(roll, Z_AXIS) * glm::rotate(pitch, X_AXIS) * glm::rotate(yaw, Y_AXIS);
}

EulerTransform3D::EulerTransform3D(types::FVec3 const &shift,
                                   types::FMat3 const &rotationMatrix,
                                   types::FVec3 const &scale)
    : _rotationMatrix(rotationMatrix)
    , _shift(shift)
    , _scale(scale)
{
}

std::unique_ptr<Transform3D> EulerTransform3D::rotate(types::FVec3 const &eulerAngles) const
{
    types::FMat3 newRotationMatrix =
        types::FMat4(_rotationMatrix) * glm::rotate(eulerAngles.z, Z_AXIS) *
        glm::rotate(eulerAngles.x, X_AXIS) * glm::rotate(eulerAngles.y, Y_AXIS);
    return std::make_unique<EulerTransform3D>(_shift, newRotationMatrix, _scale);
}

std::unique_ptr<Transform3D> EulerTransform3D::rotateAround(float angle,
                                                            types::FVec3 const &axis) const
{
    types::FMat3 newRotationMatrix = _rotationMatrix * types::FMat3(glm::rotate(angle, axis));
    return std::make_unique<EulerTransform3D>(_shift, newRotationMatrix, _scale);
}

std::unique_ptr<Transform3D> EulerTransform3D::shift(types::FVec3 const &shift) const
{
    types::FVec3 newShift = _shift + shift;
    return std::make_unique<EulerTransform3D>(newShift, _rotationMatrix, _scale);
}

std::unique_ptr<Transform3D> EulerTransform3D::scale(types::FVec3 const &scale) const
{
    types::FVec3 newScale = _scale * scale;
    return std::make_unique<EulerTransform3D>(_shift, _rotationMatrix, newScale);
}

std::unique_ptr<Transform3D> EulerTransform3D::lookAt(types::FVec3 const &lookAtTarget) const
{
    types::FVec3 forward = glm::normalize(_shift - lookAtTarget);
    types::FVec3 right = glm::cross(Y_AXIS, forward);
    types::FVec3 up = glm::cross(forward, right);
    types::FMat4 newRotationMatrix(forward.x,
                                   right.x,
                                   up.x,
                                   0,

                                   forward.y,
                                   right.y,
                                   up.y,
                                   0,

                                   forward.z,
                                   right.z,
                                   up.z,
                                   0,

                                   0,
                                   0,
                                   0,
                                   1);
    return std::make_unique<EulerTransform3D>(_shift, newRotationMatrix, _scale);
}

std::unique_ptr<Transform3D> EulerTransform3D::combine(Transform3D const &otherTransform) const
{
    return std::make_unique<EulerTransform3D>(this->transformPoint(otherTransform.shift()),
                                              _rotationMatrix * otherTransform.rotationMatrix(),
                                              _scale * otherTransform.scale());
}

types::FVec3 EulerTransform3D::transformDirection(types::FVec3 const &direction) const
{
    return _rotationMatrix * types::FVec4(direction, 0.0F);
}

types::FVec3 EulerTransform3D::transformPoint(types::FVec3 const &point) const
{
    return _shift + _rotationMatrix * (point * _scale);
}

types::FVec3 EulerTransform3D::transformVector(types::FVec3 const &vector) const
{
    return _rotationMatrix * _scale * vector;
}

types::FMat4 EulerTransform3D::localToWorldMatrix() const
{
    return glm::translate(_shift) * types::FMat4(_rotationMatrix) * glm::scale(_scale);
}

types::FVec3 EulerTransform3D::forwardVector() const
{
    return transformDirection(Z_AXIS);
}

types::FVec3 EulerTransform3D::upVector() const
{
    return transformDirection(Y_AXIS);
}

types::FVec3 EulerTransform3D::rightVector() const
{
    return transformDirection(X_AXIS);
}

types::FVec3 EulerTransform3D::shift() const
{
    return _shift;
}

types::FVec3 EulerTransform3D::scale() const
{
    return _scale;
}

types::FMat3 EulerTransform3D::rotationMatrix() const
{
    return _rotationMatrix;
}

} // namespace pf::gl
