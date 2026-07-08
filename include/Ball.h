#pragma once

#include <SFML/Graphics.hpp>

class Ball
{
public:
    Ball();

    // Basic movement, collision, and rendering methods.
    void update(float deltaTime);
    void draw(sf::RenderWindow& window) const;
    void reset();

    sf::FloatRect getBounds() const;
    sf::Vector2f getPosition() const;
    sf::Vector2f getVelocity() const;

    void setPosition(float x, float y);
    void setVelocity(float x, float y);
    void reverseX();
    void reverseY();

    // Temporary speed boost, usually applied by bumper blocks.
    void activateSpeedBoost(float multiplier, float duration);
    void updateSpeedBoost(float deltaTime);
    bool isSpeedBoosted() const;

    // Fireball status destroys special blocks and ignores ice slow.
    void activateFireball(float duration);
    void updateFireball(float deltaTime);
    void consumeFireball();
    bool isFireball() const;
    float getFireballTimer() const;

    // Prevents repeated instant teleportation between portals.
    void updateTeleportCooldown(float deltaTime);
    void triggerTeleportCooldown(float duration);
    bool canTeleport() const;

    // Slow status is applied by ice blocks and removed by bumpers or fire clouds.
    void activateSlow(float multiplier, float duration);
    void updateSlow(float deltaTime);
    void consumeSlow();
    bool isSlowed() const;

    // Delayed direction change triggered by ghost blocks.
    void updateTrajectoryShift(float deltaTime);
    void triggerDelayedTrajectoryShift(float delay);
    bool canTriggerGhostBlock() const;
   
    bool hasPendingTrajectoryShift() const;
    float getTrajectoryShiftTimer() const;

    // Allows extra balls to use different visual colors.
    void setVisualColors(
        sf::Color coreColor,
        sf::Color innerGlowColor,
        sf::Color outerGlowColor
    );

private:
    // Ball shape and current velocity.
    sf::CircleShape shape;
    sf::Vector2f velocity;

    // Base colors used for normal and extra ball rendering.
    sf::Color coreColor = sf::Color(40, 90, 255);
    sf::Color innerGlowColor = sf::Color(120, 180, 255, 90);
    sf::Color outerGlowColor = sf::Color(80, 140, 255, 40);

    // Speed boost state.
    bool speedBoosted = false;
    float speedBoostTimer = 0.f;
    float speedBoostMultiplier = 1.f;

    // Fireball state.
    bool fireballActive = false;
    float fireballTimer = 0.f;

    // Portal cooldown state.
    float teleportCooldownTimer = 0.f;

    // Ice slow state.
    bool slowed = false;
    float slowTimer = 0.f;
    float slowMultiplier = 1.f;

    // Pending ghost block trajectory shift state.
    bool trajectoryShiftPending = false;
    float trajectoryShiftTimer = 0.f;
    float ghostBlockCooldownTimer = 0.f;
};
