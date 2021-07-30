#include <cmath>

#include <glm/glm.hpp>
#include <gtest/gtest.h>

#include <pf_utils/TemplateMath.hpp>

float const ABSOLUTE_ERROR = 1e-6F;

// * length tests *

// NOLINTNEXTLINE
TEST(TemplateMath_Length, Vector2Argument_ReturnsCorrectLength)
{
    glm::vec2 vector{1.0F, 2.0F};

    float referenceLength = std::sqrt(vector.x * vector.x + vector.y * vector.y);
    auto actualLength = pf::util::tmath::length<float, glm::vec2>(vector);

    EXPECT_NEAR(referenceLength, actualLength, ABSOLUTE_ERROR);
}

// NOLINTNEXTLINE
TEST(TemplateMath_Length, Vector3Argument_ReturnsCorrectLength)
{
    glm::vec3 vector{1.0F, 2.0F, 3.0F};

    float referenceLength =
        std::sqrt(vector.x * vector.x + vector.y * vector.y + vector.z * vector.z);

    auto actualLength = pf::util::tmath::length<float, glm::vec3>(vector);
    EXPECT_NEAR(referenceLength, actualLength, ABSOLUTE_ERROR);
}

// NOLINTNEXTLINE
TEST(TemplateMath_Length, Vector4Argument_ReturnsCorrectLength)
{
    glm::vec4 vector{1.0F, 2.0F, 3.0F, 4.0F};

    float referenceLength = std::sqrt(vector.x * vector.x + vector.y * vector.y +
                                      vector.z * vector.z + vector.w * vector.w);

    auto actualLength = pf::util::tmath::length<float, glm::vec4>(vector);
    EXPECT_NEAR(referenceLength, actualLength, ABSOLUTE_ERROR);
}


// * moveToward float tests *

// NOLINTNEXTLINE
TEST(TemplateMath_MoveTowardFloat, ZeroDeltaArgument_ReturnsStartValue)
{
    float start = 1.0F;
    float target = 2.0F;

    float movedValue = pf::util::tmath::moveToward(start, target, 0.0F);

    EXPECT_EQ(movedValue, start);
}

// NOLINTNEXTLINE
TEST(TemplateMath_MoveTowardFloat, NegativeDeltaArgument_ReturnsStartValue)
{
    float start = 1.0F;
    float target = 2.0F;

    float movedValue = pf::util::tmath::moveToward(start, target, -1.0F);

    EXPECT_EQ(movedValue, start);
}

// NOLINTNEXTLINE
TEST(TemplateMath_MoveTowardFloat, LargeDeltaArgument_ReturnsTargetValue)
{
    float start = 1.0F;
    float target = 5.0F;

    float movedValue = pf::util::tmath::moveToward(start, target, 6.0F);

    EXPECT_EQ(movedValue, target);
}

// NOLINTNEXTLINE
TEST(TemplateMath_MoveTowardFloat, NormalDeltaArgument_ReturnsIntermediateValue)
{
    float start = 1.0F;
    float target = 5.0F;
    float delta = 0.5F;

    float movedValue = pf::util::tmath::moveToward(start, target, delta);

    EXPECT_NEAR(movedValue, start + delta, ABSOLUTE_ERROR);
}

// NOLINTNEXTLINE
TEST(TemplateMath_MoveTowardFloat, StartAndTargetArgumentsReversed_ReturnsIntermediateValue)
{
    float start = 5.0F;
    float target = 1.0F;
    float delta = 0.5F;

    float movedValue = pf::util::tmath::moveToward(start, target, delta);

    EXPECT_NEAR(movedValue, start - delta, ABSOLUTE_ERROR);
}


// * moveToward vector tests *

// NOLINTNEXTLINE
TEST(TemplateMath_MoveTowardVector, ZeroDeltaArgument_ReturnsStartVector)
{
    glm::vec2 start(1.0F, 2.0F);
    glm::vec2 target(4.0F, -2.0F);
    float delta = 0.0F;

    glm::vec2 movedValue = pf::util::tmath::moveToward(start, target, delta);

    EXPECT_EQ(movedValue, start);
}

// NOLINTNEXTLINE
TEST(TemplateMath_MoveTowardVector, NegativeDeltaArgument_ReturnsStartValue)
{
    glm::vec2 start(1.0F, 2.0F);
    glm::vec2 target(4.0F, -2.0F);
    float delta = -5.0F;

    glm::vec2 movedValue = pf::util::tmath::moveToward(start, target, delta);

    EXPECT_EQ(movedValue, start);
}

// NOLINTNEXTLINE
TEST(TemplateMath_MoveTowardVector, LargeDeltaArgument_ReturnsTargetValue)
{
    glm::vec2 start(0.0F, 3.0F);
    glm::vec2 target(4.0F, 0.0F);
    float delta = 6.0F;

    glm::vec2 movedValue = pf::util::tmath::moveToward(start, target, delta);

    EXPECT_EQ(movedValue, target);
}

// NOLINTNEXTLINE
TEST(TemplateMath_MoveTowardVector, NormalDeltaArgument_ReturnsIntermediateValue)
{
    glm::vec2 start(0.0F, 3.0F);
    glm::vec2 target(4.0F, 0.0F);
    float delta = glm::length(start - target) / 4.0F;

    glm::vec2 movedValue = pf::util::tmath::moveToward(start, target, delta);

    EXPECT_NEAR(movedValue.x, 1.0F, ABSOLUTE_ERROR);
    EXPECT_NEAR(movedValue.y, 2.25F, ABSOLUTE_ERROR);
}
