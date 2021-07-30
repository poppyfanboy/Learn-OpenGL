#include <glm/glm.hpp>
#include <gtest/gtest.h>

#include <pf_utils/VectorMath.hpp>

float const ABSOLUTE_ERROR = 1e-6F;

// NOLINTNEXTLINE
TEST(VectorMath_MoveToward, ZeroDeltaArgument_ReturnsStartVector)
{
    glm::vec2 start(1.0F, 2.0F);
    glm::vec2 target(4.0F, -2.0F);
    float delta = 0.0F;

    glm::vec2 movedValue = pf::util::math::moveToward(start, target, delta);

    EXPECT_EQ(movedValue, start);
}

// NOLINTNEXTLINE
TEST(VectorMath_MoveToward, NegativeDeltaArgument_ReturnsStartValue)
{
    glm::vec2 start(1.0F, 2.0F);
    glm::vec2 target(4.0F, -2.0F);
    float delta = -5.0F;

    glm::vec2 movedValue = pf::util::math::moveToward(start, target, delta);

    EXPECT_EQ(movedValue, start);
}

// NOLINTNEXTLINE
TEST(VectorMath_MoveToward, LargeDeltaArgument_ReturnsTargetValue)
{
    glm::vec2 start(0.0F, 3.0F);
    glm::vec2 target(4.0F, 0.0F);
    float delta = 6.0F;

    glm::vec2 movedValue = pf::util::math::moveToward(start, target, delta);

    EXPECT_EQ(movedValue, target);
}

// NOLINTNEXTLINE
TEST(VectorMath_MoveToward, NormalDeltaArgument_ReturnsIntermediateValue)
{
    glm::vec2 start(0.0F, 3.0F);
    glm::vec2 target(4.0F, 0.0F);
    float delta = 1.25F;

    glm::vec2 movedValue = pf::util::math::moveToward(start, target, delta);

    EXPECT_NEAR(movedValue.x, 1.0F, ABSOLUTE_ERROR);
    EXPECT_NEAR(movedValue.y, 2.25F, ABSOLUTE_ERROR);
}
