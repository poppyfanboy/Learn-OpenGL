#include <pf_gl/MinecraftCamera.hpp>

#include <cmath>
#include <algorithm>
#include <stdexcept>
#include <numbers>

#include <glm/glm.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <glm/ext/matrix_clip_space.hpp>
#include <glad/glad.h>

#include <pf_gl/ValueTypes.hpp>
#include <pf_utils/VectorMath.hpp>

namespace pf::gl
{

types::Float const MinecraftCamera::MIN_FOV = std::numbers::pi / 128.0F;
types::Float const MinecraftCamera::MAX_FOV = std::numbers::pi - 1e-2F;
types::Float const MinecraftCamera::CAMERA_ROTATION_EPSILON = 1e-2F;
types::Float const MinecraftCamera::MAX_MOVEMENT_VELOCITY = 8.0F;
types::Float const MinecraftCamera::MOVEMENT_ACCELERATION = 24.0F;
types::Float const MinecraftCamera::MOVEMENT_FRICTION = 24.0F;
types::FVec3 const MinecraftCamera::WORLD_UP = types::FVec3(0.0F, 1.0F, 0.0F);
types::FVec3 const MinecraftCamera::WORLD_RIGHT = types::FVec3(1.0F, 0.0F, 0.0F);

MinecraftCamera::Builder &MinecraftCamera::Builder::withAspectRatio(types::Float aspectRatio)
{
    _aspectRatio = aspectRatio;
    return *this;
}

MinecraftCamera::Builder &MinecraftCamera::Builder::atPosition(types::FVec3 position)
{
    _position = position;
    return *this;
}

MinecraftCamera::Builder &MinecraftCamera::Builder::withFarClippingAt(types::Float farClipping)
{
    _farClipping = farClipping;
    return *this;
}

MinecraftCamera::Builder &MinecraftCamera::Builder::withNearClippingAt(types::Float nearClipping)
{
    _nearClipping = nearClipping;
    return *this;
}

MinecraftCamera::Builder &MinecraftCamera::Builder::withYaw(types::Float yaw)
{
    _yaw = yaw;
    return *this;
}

MinecraftCamera::Builder &MinecraftCamera::Builder::withPitch(types::Float pitch)
{
    _pitch = pitch;
    return *this;
}

MinecraftCamera::Builder &MinecraftCamera::Builder::withRotationSpeed(types::Float rotationSpeed)
{
    _rotationSpeed = rotationSpeed;
    return *this;
}

MinecraftCamera::Builder &MinecraftCamera::Builder::withFOV(types::Float fov)
{
    _fov = fov;
    return *this;
}

MinecraftCamera MinecraftCamera::Builder::build()
{
    return MinecraftCamera(
        _aspectRatio, _position, _yaw, _pitch, _nearClipping, _farClipping, _rotationSpeed, _fov);
}

MinecraftCamera::MinecraftCamera(types::Float aspectRatio,
                                 types::FVec3 const &position,
                                 types::Float yaw,
                                 types::Float pitch,
                                 types::Float nearClipping,
                                 types::Float farClipping,
                                 types::Float rotationSpeed,
                                 types::Float fov)
    : _aspectRatio(aspectRatio)
    , _position(position)
    , _yaw(yaw)
    , _pitch(pitch)
    , _nearClipping(nearClipping)
    , _farClipping(farClipping)
    , _rotationSpeed(rotationSpeed)
    , _fov(fov)
{
    if (_aspectRatio <= 0.0F)
    {
        throw std::invalid_argument("Aspect ratio must be positive.");
    }
    if (_nearClipping <= 0.0F || _farClipping <= 0.0F)
    {
        throw std::invalid_argument("Clipping distance must be positive");
    }
    if (_nearClipping >= _farClipping)
    {
        throw std::invalid_argument(
            "Far clipping distance must be larger than near clipping distance.");
    }
    if (_rotationSpeed <= 0.0F)
    {
        throw std::invalid_argument("Rotation speed must be positive.");
    }
    _fov = std::clamp(_fov, MIN_FOV, MAX_FOV);

    updateViewMatrix();
    _projectionMatrix = glm::perspective(_fov, _aspectRatio, _nearClipping, _farClipping);
}

void MinecraftCamera::updateViewMatrix()
{
    _front = types::DEFAULT_VALUE<types::FVec3>;
    _front.x = -std::cos(_pitch) * std::cos(_yaw);
    _front.y = -std::sin(_pitch);
    _front.z = -std::cos(_pitch) * std::sin(_yaw);
    _front = glm::normalize(_front);

    _right = glm::normalize(glm::cross(WORLD_UP, _front));
    _up = glm::cross(_front, _right);

    _viewMatrix = types::DEFAULT_VALUE<types::FMat4>;
    _viewMatrix = glm::rotate(_viewMatrix, _pitch, WORLD_RIGHT);
    _viewMatrix = glm::rotate(_viewMatrix, _yaw, WORLD_UP);
    _viewMatrix = glm::translate(_viewMatrix, -_position);
}

types::FMat4 const &MinecraftCamera::viewMatrix() const
{
    return _viewMatrix;
}

types::FMat4 const &MinecraftCamera::projectionMatrix() const
{
    return _projectionMatrix;
}

void MinecraftCamera::move(types::FVec3 const &movementInput, types::Float deltaTime)
{
    if (glm::length(movementInput) != 0.0F)
    {
        glm::vec3 normalizedMovementInput = -glm::normalize(movementInput);
        types::FVec3 forward = glm::normalize(types::FVec3(_front.x, 0.0F, _front.z));
        types::FVec3 movementDirection = normalizedMovementInput.x * _right +
                                         normalizedMovementInput.y * forward +
                                         normalizedMovementInput.z * WORLD_UP;
        _movementVelocity = pf::util::math::moveToward(_movementVelocity,
                                                       MAX_MOVEMENT_VELOCITY * movementDirection,
                                                       MOVEMENT_ACCELERATION * deltaTime);
    }
    else
    {
        _movementVelocity = pf::util::math::moveToward(
            _movementVelocity, types::FVec3(0.0F, 0.0F, 0.0F), MOVEMENT_FRICTION * deltaTime);
    }
    _position += _movementVelocity * deltaTime;
    updateViewMatrix();
}

void MinecraftCamera::rotate(types::FVec2 const &rotationInput, types::Float deltaTime)
{
    if (glm::length(rotationInput) == 0.0F)
    {
        return;
    }
    _yaw += rotationInput.x * _rotationSpeed * deltaTime;
    _pitch += rotationInput.y * _rotationSpeed * deltaTime;
    _pitch = std::clamp(_pitch,
                        static_cast<types::Float>(-std::numbers::pi / 2 + CAMERA_ROTATION_EPSILON),
                        static_cast<types::Float>(std::numbers::pi / 2 - CAMERA_ROTATION_EPSILON));
    updateViewMatrix();
}

void MinecraftCamera::fov(types::Float fov)
{
    _fov = std::clamp(fov, MIN_FOV, MAX_FOV);
    _projectionMatrix = glm::perspective(_fov, _aspectRatio, _nearClipping, _farClipping);
}

types::FVec3 const &MinecraftCamera::position() const
{
    return _position;
}

types::FVec3 MinecraftCamera::direction() const
{
    return -_front;
}

void MinecraftCamera::aspectRatio(types::Float aspectRatio)
{
    _aspectRatio = std::max(0.0F, aspectRatio);
    _projectionMatrix = glm::perspective(_fov, _aspectRatio, _nearClipping, _farClipping);
}

} // namespace pf::gl
