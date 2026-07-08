#include "Ball.h"
#include "Constants.h"
#include <cmath>
#include <cstdint>

// Initializes the ball shape, origin, and starting velocity.
Ball::Ball()
{
    shape.setRadius(Constants::BallRadius);
    shape.setOrigin({Constants::BallRadius, Constants::BallRadius});
    shape.setFillColor(coreColor);
    reset();
}

// Resets position, velocity, and all temporary ball effects.
void Ball::reset()
{
    shape.setPosition({Constants::BallStartX, Constants::BallStartY});
    velocity = {90.f, Constants::BallStartSpeedY};

    speedBoosted = false;
    speedBoostTimer = 0.f;
    speedBoostMultiplier = 1.f;

    fireballActive = false;
    fireballTimer = 0.f;

    slowed = false;
    slowTimer = 0.f;
    slowMultiplier = 1.f;

    teleportCooldownTimer = 0.f;

    trajectoryShiftPending = false;
    trajectoryShiftTimer = 0.f;
    ghostBlockCooldownTimer = 0.f;
}

void Ball::update(float deltaTime)
{
    shape.move(velocity * deltaTime);
}

void Ball::draw(sf::RenderWindow& window) const
{
    sf::CircleShape shadow(Constants::BallRadius);
    shadow.setOrigin({ Constants::BallRadius, Constants::BallRadius });
    shadow.setPosition({
        shape.getPosition().x + Constants::ShadowOffsetX,
        shape.getPosition().y + Constants::ShadowOffsetY
    });
    shadow.setScale({ 1.25f, 0.85f });
    shadow.setFillColor(sf::Color(0, 0, 0, 70));

    sf::CircleShape outerGlow(Constants::BallRadius + Constants::GlowSizeMedium);
    outerGlow.setOrigin({
        Constants::BallRadius + Constants::GlowSizeMedium,
        Constants::BallRadius + Constants::GlowSizeMedium
    });
    outerGlow.setPosition(shape.getPosition());

    sf::CircleShape innerGlow(Constants::BallRadius + Constants::GlowSizeSmall);
    innerGlow.setOrigin({
        Constants::BallRadius + Constants::GlowSizeSmall,
        Constants::BallRadius + Constants::GlowSizeSmall
    });
    innerGlow.setPosition(shape.getPosition());

    // Choose glow colors based on the strongest active visual status.
    if (fireballActive)
    {
        outerGlow.setFillColor(sf::Color(255, 75, 25, 100));
        innerGlow.setFillColor(sf::Color(255, 180, 70, 150));
    }
    else if (slowed)
    {
        outerGlow.setFillColor(sf::Color(110, 220, 255, 95));
        innerGlow.setFillColor(sf::Color(210, 250, 255, 145));
    }
    else if (speedBoosted)
    {
        outerGlow.setFillColor(sf::Color(255, 210, 90, 65));
        innerGlow.setFillColor(sf::Color(255, 235, 120, 110));
    }
    else
    {
        outerGlow.setFillColor(outerGlowColor);
        innerGlow.setFillColor(innerGlowColor);
    }

    // Draw a fire trail behind the ball while fireball mode is active.
    if (fireballActive)
    {
        sf::Vector2f direction = velocity;

        const float length = std::sqrt(direction.x * direction.x + direction.y * direction.y);

        if (length > 0.f)
        {
            direction.x /= length;
            direction.y /= length;
        }

        const sf::Vector2f oppositeDirection{
            -direction.x,
            -direction.y
        };

        for (int i = 1; i <= 5; ++i)
        {
            const float distance = static_cast<float>(i) * 10.f;
            const float radius = Constants::BallRadius + static_cast<float>(6 - i) * 2.f;
            const std::uint8_t alpha = static_cast<std::uint8_t>(80 - i * 12);

            sf::CircleShape trail(radius);
            trail.setOrigin({ radius, radius });

            trail.setPosition({
                shape.getPosition().x + oppositeDirection.x * distance,
                shape.getPosition().y + oppositeDirection.y * distance
            });

            trail.setFillColor(sf::Color(255, 95, 35, alpha));

            window.draw(trail);
        }
    }

    // Draw a frost trail while the ball is slowed.
    if (slowed && !fireballActive)
    {
        sf::Vector2f direction = velocity;

        const float length = std::sqrt(direction.x * direction.x + direction.y * direction.y);

        if (length > 0.f)
        {
            direction.x /= length;
            direction.y /= length;
        }

        const sf::Vector2f oppositeDirection{
            -direction.x,
            -direction.y
        };

        for (int i = 1; i <= 4; ++i)
        {
            const float distance = static_cast<float>(i) * 8.f;
            const float radius = Constants::BallRadius + static_cast<float>(5 - i) * 1.4f;
            const std::uint8_t alpha =
                static_cast<std::uint8_t>(70 - i * 12);

            sf::CircleShape frostTrail(radius);
            frostTrail.setOrigin({ radius, radius });

            frostTrail.setPosition({
                shape.getPosition().x + oppositeDirection.x * distance,
                shape.getPosition().y + oppositeDirection.y * distance
            });

            frostTrail.setFillColor(sf::Color(170, 235, 255, alpha));

            window.draw(frostTrail);
        }
    }

    // Add a small icy core on top of the slowed ball.
    if (slowed && !fireballActive)
    {
        sf::CircleShape frostCore(Constants::BallRadius * 0.42f);
        frostCore.setOrigin({
            Constants::BallRadius * 0.42f,
            Constants::BallRadius * 0.42f
        });
        frostCore.setPosition(shape.getPosition());
        frostCore.setFillColor(sf::Color(240, 255, 255, 150));

        window.draw(frostCore);
    }

    window.draw(shadow);
    window.draw(outerGlow);
    window.draw(innerGlow);
    window.draw(shape);

    // Show a warning ring while a delayed trajectory shift is pending.
    if (trajectoryShiftPending)
    {
        const float timerRatio =
            trajectoryShiftTimer / Constants::GhostBlockTrajectoryDelay;

        const float pulse =
            1.f + 0.18f * std::sin(trajectoryShiftTimer * 18.f);

        const float ringRadius =
            Constants::BallRadius * (1.75f + (1.f - timerRatio) * 0.45f) * pulse;

        const std::uint8_t alpha =
            static_cast<std::uint8_t>(160.f * timerRatio);

        sf::CircleShape warningRing(ringRadius);
        warningRing.setOrigin({ ringRadius, ringRadius });
        warningRing.setPosition(shape.getPosition());
        warningRing.setFillColor(sf::Color::Transparent);
        warningRing.setOutlineThickness(2.f);
        warningRing.setOutlineColor(sf::Color(190, 240, 255, alpha));

        window.draw(warningRing);

        sf::CircleShape energyCore(Constants::BallRadius * 0.35f);
        energyCore.setOrigin({
            Constants::BallRadius * 0.35f,
            Constants::BallRadius * 0.35f
        });
        energyCore.setPosition(shape.getPosition());
        energyCore.setFillColor(sf::Color(
            230,
            255,
            255,
            static_cast<std::uint8_t>(90.f * timerRatio)
        ));

        window.draw(energyCore);
    }
}

sf::FloatRect Ball::getBounds() const
{
    return shape.getGlobalBounds();
}

sf::Vector2f Ball::getPosition() const
{
    return shape.getPosition();
}

sf::Vector2f Ball::getVelocity() const
{
    return velocity;
}

void Ball::setPosition(float x, float y)
{
    shape.setPosition({x, y});
}

void Ball::setVelocity(float x, float y)
{
    velocity = {x, y};
}

void Ball::reverseX()
{
    velocity.x = -velocity.x;
}

void Ball::reverseY()
{
    velocity.y = -velocity.y;
}

// Overrides default colors, mainly used for extra balls.
void Ball::setVisualColors(
    sf::Color newCoreColor,
    sf::Color newInnerGlowColor,
    sf::Color newOuterGlowColor
)
{
    coreColor = newCoreColor;
    innerGlowColor = newInnerGlowColor;
    outerGlowColor = newOuterGlowColor;

    shape.setFillColor(coreColor);
}

// Applies a temporary velocity multiplier.
void Ball::activateSpeedBoost(float multiplier, float duration)
{
    if (speedBoosted)
    {
        speedBoostTimer = duration;
        return;
    }

    speedBoosted = true;
    speedBoostTimer = duration;
    speedBoostMultiplier = multiplier;

    velocity.x *= speedBoostMultiplier;
    velocity.y *= speedBoostMultiplier;
}

// Restores normal speed when the boost timer expires.
void Ball::updateSpeedBoost(float deltaTime)
{
    if (!speedBoosted)
        return;

    speedBoostTimer -= deltaTime;

    if (speedBoostTimer <= 0.f)
    {
        speedBoosted = false;
        speedBoostTimer = 0.f;

        velocity.x /= speedBoostMultiplier;
        velocity.y /= speedBoostMultiplier;

        speedBoostMultiplier = 1.f;
    }
}

bool Ball::isSpeedBoosted() const
{
    return speedBoosted;
}

// Enables fireball mode without changing the current velocity.
void Ball::activateFireball(float duration)
{
    fireballActive = true;
    fireballTimer = duration;
}

void Ball::updateFireball(float deltaTime)
{
    if (!fireballActive)
        return;

    fireballTimer -= deltaTime;

    if (fireballTimer <= 0.f)
    {
        consumeFireball();
    }
}

// Removes fireball mode after it is consumed by a valid interaction.
void Ball::consumeFireball()
{
    fireballActive = false;
    fireballTimer = 0.f;
}

bool Ball::isFireball() const
{
    return fireballActive;
}

float Ball::getFireballTimer() const
{
    return fireballTimer;
}

// Counts down the portal cooldown timer.
void Ball::updateTeleportCooldown(float deltaTime)
{
    if (teleportCooldownTimer <= 0.f)
        return;

    teleportCooldownTimer -= deltaTime;

    if (teleportCooldownTimer < 0.f)
        teleportCooldownTimer = 0.f;
}

// Starts cooldown after teleporting through a portal.
void Ball::triggerTeleportCooldown(float duration)
{
    teleportCooldownTimer = duration;
}

bool Ball::canTeleport() const
{
    return teleportCooldownTimer <= 0.f;
}

// Applies ice slow by scaling the current velocity.
void Ball::activateSlow(float multiplier, float duration)
{
    if (slowed)
    {
        slowTimer = duration;
        return;
    }

    slowed = true;
    slowTimer = duration;
    slowMultiplier = multiplier;

    velocity.x *= slowMultiplier;
    velocity.y *= slowMultiplier;
}

// Updates the slow timer and restores velocity when it expires.
void Ball::updateSlow(float deltaTime)
{
    if (!slowed)
        return;

    slowTimer -= deltaTime;

    if (slowTimer <= 0.f)
    {
        consumeSlow();
    }
}

// Removes slow immediately and restores the previous velocity scale.
void Ball::consumeSlow()
{
    if (!slowed)
        return;

    slowed = false;
    slowTimer = 0.f;

    velocity.x /= slowMultiplier;
    velocity.y /= slowMultiplier;

    slowMultiplier = 1.f;
}

bool Ball::isSlowed() const
{
    return slowed;
}

// Schedules a delayed horizontal direction change from a ghost block.
void Ball::triggerDelayedTrajectoryShift(float delay)
{
    if (trajectoryShiftPending)
        return;

    if (ghostBlockCooldownTimer > 0.f)
        return;

    trajectoryShiftPending = true;
    trajectoryShiftTimer = delay;
    ghostBlockCooldownTimer = Constants::GhostBlockTriggerCooldown;
}

// Applies the delayed trajectory change once the timer reaches zero.
void Ball::updateTrajectoryShift(float deltaTime)
{
    if (ghostBlockCooldownTimer > 0.f)
    {
        ghostBlockCooldownTimer -= deltaTime;

        if (ghostBlockCooldownTimer < 0.f)
            ghostBlockCooldownTimer = 0.f;
    }

    if (!trajectoryShiftPending)
        return;

    trajectoryShiftTimer -= deltaTime;

    if (trajectoryShiftTimer > 0.f)
        return;

    trajectoryShiftPending = false;
    trajectoryShiftTimer = 0.f;

    velocity.x = -velocity.x;

    if (std::abs(velocity.x) < 140.f)
    {
        if (velocity.x < 0.f)
            velocity.x = -180.f;
        else
            velocity.x = 180.f;
    }
}

bool Ball::canTriggerGhostBlock() const
{
    return !trajectoryShiftPending && ghostBlockCooldownTimer <= 0.f;
}

bool Ball::hasPendingTrajectoryShift() const
{
    return trajectoryShiftPending;
}

float Ball::getTrajectoryShiftTimer() const
{
    return trajectoryShiftTimer;
}