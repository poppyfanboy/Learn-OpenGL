#ifndef MINECRAFT_CAMERA_HPP
#define MINECRAFT_CAMERA_HPP

#include <cmath>
#include <numbers>

#include <pf_gl/ValueTypes.hpp>

namespace pf::gl
{

/**
 * A simple camera with Minecraft-like controls.
 */
class MinecraftCamera
{
public:
    class Builder
    {
    public:
        Builder &withAspectRatio(types::Float aspectRatio);
        Builder &atPosition(types::FVec3 position);
        Builder &withFarClippingAt(types::Float farClipping);
        Builder &withNearClippingAt(types::Float nearClipping);
        Builder &withYaw(types::Float yaw);
        Builder &withPitch(types::Float pitch);
        Builder &withRotationSpeed(types::Float rotationSpeed);
        Builder &withFOV(types::Float fov);

        MinecraftCamera build();

    private:
        types::Float _aspectRatio = 16.0F / 9.0F;
        types::FVec3 _position = types::DEFAULT_VALUE<types::FVec3>;
        types::Float _nearClipping = 0.1F, _farClipping = 100.0F;
        types::Float _yaw = 0.0F, _pitch = 0.0F;
        types::Float _rotationSpeed = 0.15F;
        types::Float _fov = std::numbers::pi / 4.0F;
    };

    MinecraftCamera(types::Float aspectRatio,
                    types::FVec3 const &position,
                    types::Float yaw,
                    types::Float pitch,
                    types::Float nearClippingDistance,
                    types::Float farClippingDistance,
                    types::Float rotationSpeed,
                    types::Float fov);

    [[nodiscard]] types::FMat4 const &viewMatrix() const;
    [[nodiscard]] types::FMat4 const &projectionMatrix() const;
    [[nodiscard]] types::FVec3 const &position() const;
    [[nodiscard]] types::FVec3 direction() const;

    void move(types::FVec3 const &movementInput, types::Float deltaTime);
    void rotate(types::FVec2 const &rotationInput, types::Float deltaTime);
    void fov(types::Float fov);
    void aspectRatio(types::Float aspectRatio);

private:
    static types::Float const MIN_FOV, MAX_FOV;
    static types::Float const MAX_MOVEMENT_VELOCITY, MOVEMENT_ACCELERATION, MOVEMENT_FRICTION;
    static types::FVec3 const WORLD_UP, WORLD_RIGHT;

    /**
     * Clamps pitch at (-PI / 2 + EPS, PI / 2 - EPS), so that pitch is never exactly (PI / 2) or
     * (- PI / 2).
     */
    static types::Float const CAMERA_ROTATION_EPSILON;

    // * View *

    types::FMat4 _viewMatrix;
    types::Float _yaw, _pitch;
    types::FVec3 _position;
    types::FVec3 _front, _up, _right;

    // * Projection *

    types::Float _aspectRatio;
    types::Float _fov;
    types::Float _nearClipping, _farClipping;
    types::FMat4 _projectionMatrix;

    types::Float _rotationSpeed;
    types::FVec3 _movementVelocity = types::DEFAULT_VALUE<types::FVec3>;

    void updateViewMatrix();
};

} // namespace pf::gl

#endif // !MINECRAFT_CAMERA_HPP
