#pragma once

#include <SFML/Graphics.hpp>

class Brick
{
public:
    // Creates a block from a level type and optional link group.
    Brick(sf::Vector2f position, int type, int linkGroup = -1);

    // Basic rendering, collision, and state methods.
    void draw(sf::RenderWindow& window) const;
    sf::FloatRect getBounds() const;

    void hit();
    void destroy();
    void hitByFireball();

    // Block type and state checks.
    bool isDestroyed() const;
    bool isMetal() const;
    bool isMoving() const;
    bool isBumper() const;
    bool isDamagedMetal() const;
    bool isPortal() const;
    bool isIce() const;
    bool isGenerator() const;
    bool isGhostBlock() const;

    int getType() const;

    // Portal and linked block group data.
    int getPortalGroup() const;
    sf::Color getPortalPrimaryColor() const;
    sf::Color getPortalSecondaryColor() const;

    int getLinkGroup() const;

    // Movement control for moving metal blocks.
    float getMoveDirection() const;
    float getMoveSpeed() const;

    void setMoveDirection(float direction);
    void move(sf::Vector2f offset);

    // Temporary visual effects for special block interactions.
    void updateHitEffect(float deltaTime);
    void triggerMetalHitEffect();

    void updateBumper(float deltaTime);
    void triggerBumperHitEffect();

    void updatePortalAnimation(float deltaTime);
    void triggerPortalFlash();
    void updatePortalFlash(float deltaTime);

    void triggerGhostFlash();
    void updateGhostFlash(float deltaTime);

private:
    // Shared collision and rendering shape.
    sf::RectangleShape shape;

    // Type, health, and destruction state.
    int type = 0;
    int hitPoints = 1;
    bool destroyed = false;

    // Moving metal block state.
    bool moving = false;
    float moveDirection = 1.f;
    float moveSpeed = 80.f;

    // Fireball damage state for metal blocks.
    int metalDamageLevel = 0;

    // Generic hit flash or shake timer.
    float hitEffectTimer = 0.f;

    // Bumper block movement and hit effect state.
    bool bumper = false;
    float bumperStartY = 0.f;
    float bumperDirection = 1.f;
    float bumperHitEffectTimer = 0.f;

    // Shared visual timers for portal and ghost block effects.
    float portalAnimationTimer = 0.f;
    float portalPhase = 0.f;
    float portalFlashTimer = 0.f;
    float ghostFlashTimer = 0.f;

    // Connects generators with their matching ghost blocks.
    int linkGroup = -1;
};
