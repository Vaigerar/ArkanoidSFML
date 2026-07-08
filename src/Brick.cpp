#include "Brick.h"
#include "Constants.h"
#include "cmath"
#include <algorithm>
#include <cstdint>

// Configures block behavior, health, color, and optional link group.
Brick::Brick(sf::Vector2f position, int type, int linkGroup)
    : type(type), linkGroup(linkGroup)
{
    shape.setSize({Constants::BrickWidth - 1.f, Constants::BrickHeight - 1.f});
    shape.setPosition(position);

    // Level type defines the block mechanics.
    switch (type)
    {
    case 1:
        hitPoints = 1;
        shape.setFillColor(sf::Color(120, 180, 255));
        break;
    case 2:
        hitPoints = 1;
        shape.setFillColor(sf::Color(120, 255, 160));
        break;
    case 3:
        hitPoints = 2;
        shape.setFillColor(sf::Color(255, 180, 90));
        break;
    case 4:
        hitPoints = 1;
        shape.setFillColor(sf::Color(255, 110, 110));
        break;
    case 5:
        // Moving metal block. It can only be damaged by fireballs.
        hitPoints = 9999;
        moving = true;
        moveSpeed = Constants::MetalBlockMoveSpeed;
        shape.setFillColor(sf::Color(170, 180, 195));
        break;
    case 6:     
    {
        // Bumper block. It boosts the ball and is not destructible.
        hitPoints = 9999;
        bumper = true;

        shape.setSize({
            Constants::BumperWidth,
            Constants::BumperHeight
        });

        shape.setPosition({
            position.x + (Constants::BrickWidth - Constants::BumperWidth) / 2.f,
            position.y - Constants::BrickHeight
        });

        bumperStartY = shape.getPosition().y;

        shape.setFillColor(sf::Color(255, 90, 190));
        break;
    }
    case 7:
        // Portal blocks teleport the ball between portals of the same type.
        hitPoints = 9999;
        shape.setFillColor(sf::Color(70, 45, 120));
        portalPhase = position.x * 0.01f + position.y * 0.005f;
        break;
    case 8:
        hitPoints = 9999;
        shape.setFillColor(sf::Color(35, 80, 120));
        portalPhase = position.x * 0.012f + position.y * 0.006f;
        break;
    case 9:
        hitPoints = 9999;
        shape.setFillColor(sf::Color(120, 45, 95));
        portalPhase = position.x * 0.014f + position.y * 0.004f;
        break;
    case 10:
        // Ice block. Applies slow when destroyed by a non-fireball hit.
        hitPoints = 1;
        shape.setFillColor(sf::Color(125, 220, 245));
        break;
    case 11:
        // Generator block. Explodes and removes linked ghost blocks.
        hitPoints = 1;
        shape.setFillColor(sf::Color(255, 210, 80));
        break;
    case 12:
        // Ghost block. Pass-through trigger linked to a generator.
        hitPoints = 9999;
        shape.setFillColor(sf::Color(180, 220, 255, 80));
        break;
    default:
        hitPoints = 0;
        break;
    }
}

// Draws the block with type-specific visual effects.
void Brick::draw(sf::RenderWindow& window) const
{
    // Destroyed blocks are skipped completely.
    if (isDestroyed())
        return;

    float shakeOffsetX = 0.f;

    // Add a short shake effect when metal is hit.
    if (hitEffectTimer > 0.f && isMetal())
    {
        shakeOffsetX = std::sin(hitEffectTimer * 90.f) * 3.f;
    }

    float bumperScale = 1.f;

    // Add a pulse effect when the bumper is activated.
    if (bumperHitEffectTimer > 0.f && isBumper())
    {
        bumperScale = 1.f + bumperHitEffectTimer * 0.8f;
    }

    // Copy the collision shape so visual effects do not affect gameplay bounds.
    sf::RectangleShape visualShape = shape;
    
    if (isBumper() && bumperHitEffectTimer > 0.f)
    {
        visualShape.setOrigin({
            visualShape.getSize().x / 2.f,
            visualShape.getSize().y / 2.f
        });

        visualShape.setPosition({
            shape.getPosition().x + shape.getSize().x / 2.f,
            shape.getPosition().y + shape.getSize().y / 2.f
        });

        visualShape.setScale({ 1.f, bumperScale });
    }

    visualShape.move({ shakeOffsetX, 0.f });

    sf::RectangleShape shadow = visualShape;
    shadow.move({ 3.f, 3.f });
    shadow.setFillColor(sf::Color(0, 0, 0, 80));

    window.draw(shadow);

    sf::RectangleShape glow;
    glow.setSize({
        visualShape.getSize().x + 6.f,
        visualShape.getSize().y + 6.f
    });
    glow.setPosition({
        visualShape.getPosition().x - 3.f,
        visualShape.getPosition().y - 3.f
    });

    // Choose glow color based on the visible block type.
    switch (type)
    {
    case 1:
        glow.setFillColor(sf::Color(100, 170, 255, 30));
        break;
    case 2:
        glow.setFillColor(sf::Color(120, 255, 140, 45));
        break;
    case 3:
    case 4:
        glow.setFillColor(sf::Color(255, 180, 100, 40));
        break;
    case 5:
        if (isDamagedMetal())
            glow.setFillColor(sf::Color(255, 100, 80, 55));
        else    
            glow.setFillColor(sf::Color(190, 210, 230, 45));
        break;
    case 6:
        glow.setFillColor(sf::Color(255, 90, 190, 70));
        break;
    case 7:
        glow.setFillColor(sf::Color(155, 95, 255, 85));
        break;
    case 8:
        glow.setFillColor(sf::Color(90, 220, 255, 85));
        break;
    case 9:
        glow.setFillColor(sf::Color(255, 100, 185, 85));
        break;
    case 10:
        glow.setFillColor(sf::Color(150, 240, 255, 78));
        break;
    case 11:
        glow.setFillColor(sf::Color(255, 210, 80, 80));
        break;
    case 12:
        glow.setFillColor(sf::Color(160, 220, 255, 35));
        break;
    default:
        glow.setFillColor(sf::Color(255, 255, 255, 0));
        break;
    }

    window.draw(glow);

    // Damaged metal is rendered as two separated halves.
    if (isDamagedMetal())
    {
        const sf::Vector2f position = visualShape.getPosition();
        const sf::Vector2f size = visualShape.getSize();

        const float gap = 7.f;
        const float halfWidth = (size.x - gap) / 2.f;

        sf::RectangleShape leftHalf;
        leftHalf.setSize({ halfWidth, size.y });
        leftHalf.setPosition(position);
        leftHalf.setFillColor(sf::Color(95, 105, 120));

        sf::RectangleShape rightHalf;
        rightHalf.setSize({ halfWidth, size.y });
        rightHalf.setPosition({
            position.x + halfWidth + gap,
            position.y
        });
        rightHalf.setFillColor(sf::Color(95, 105, 120));

        sf::RectangleShape leftHighlight;
        leftHighlight.setSize({ halfWidth, 1.f });
        leftHighlight.setPosition(position);
        leftHighlight.setFillColor(sf::Color(255, 255, 255, 22));

        sf::RectangleShape rightHighlight;
        rightHighlight.setSize({ halfWidth, 1.f });
        rightHighlight.setPosition({
            position.x + halfWidth + gap,
            position.y
        });
        rightHighlight.setFillColor(sf::Color(255, 255, 255, 22));

        sf::RectangleShape crackShadow;
        crackShadow.setSize({ gap, size.y });
        crackShadow.setPosition({
            position.x + halfWidth,
            position.y
        });
        crackShadow.setFillColor(sf::Color(15, 20, 30, 190));

        window.draw(leftHalf);
        window.draw(rightHalf);
        window.draw(crackShadow);
        window.draw(leftHighlight);
        window.draw(rightHighlight);
    }
    else
    {
        window.draw(visualShape);
    }

    // Draw the generator core and energy cross.
    if (isGenerator())
    {
        const sf::Vector2f position = visualShape.getPosition();
        const sf::Vector2f size = visualShape.getSize();

        sf::CircleShape core(size.y * 0.24f);
        core.setOrigin({ size.y * 0.24f, size.y * 0.24f });
        core.setPosition({
            position.x + size.x / 2.f,
            position.y + size.y / 2.f
        });
        core.setFillColor(sf::Color(255, 245, 160, 160));

        sf::RectangleShape lineH;
        lineH.setSize({ size.x * 0.65f, 2.f });
        lineH.setOrigin({ lineH.getSize().x / 2.f, 1.f });
        lineH.setPosition({
            position.x + size.x / 2.f,
            position.y + size.y / 2.f
        });
        lineH.setFillColor(sf::Color(255, 245, 180, 120));

        sf::RectangleShape lineV;
        lineV.setSize({ 2.f, size.y * 0.65f });
        lineV.setOrigin({ 1.f, lineV.getSize().y / 2.f });
        lineV.setPosition({
            position.x + size.x / 2.f,
            position.y + size.y / 2.f
        });
        lineV.setFillColor(sf::Color(255, 245, 180, 120));

        window.draw(core);
        window.draw(lineH);
        window.draw(lineV);
    }

    // Draw a translucent energy field for pass-through ghost blocks.
    if (isGhostBlock())
    {
        const sf::Vector2f position = visualShape.getPosition();
        const sf::Vector2f size = visualShape.getSize();

        const float energyPulse =
            0.75f + 0.25f * std::sin(portalAnimationTimer * 4.2f + portalPhase);

        const std::uint8_t veilAlpha =
            static_cast<std::uint8_t>(20.f * energyPulse);

        const std::uint8_t innerAlpha =
            static_cast<std::uint8_t>(12.f * energyPulse);

        const std::uint8_t coreAlpha =
            static_cast<std::uint8_t>(70.f * energyPulse);

        const std::uint8_t lineAlpha =
            static_cast<std::uint8_t>(95.f * energyPulse);

        sf::RectangleShape veil = visualShape;
        veil.setFillColor(sf::Color(180, 230, 255, veilAlpha));

        sf::RectangleShape inner;
        inner.setSize({ size.x - 10.f, size.y - 10.f });
        inner.setPosition({
            position.x + 5.f,
            position.y + 5.f
        });
        inner.setFillColor(sf::Color(220, 250, 255, innerAlpha));

        sf::CircleShape energyCore(size.y * 0.14f);
        energyCore.setOrigin({ size.y * 0.14f, size.y * 0.14f });
        energyCore.setPosition({
            position.x + size.x / 2.f,
            position.y + size.y / 2.f
        });
        energyCore.setScale({ 1.6f, 0.75f });
        energyCore.setFillColor(sf::Color(235, 255, 255, coreAlpha));

        sf::RectangleShape energyLine1;
        energyLine1.setSize({ size.x * 0.70f, 1.5f });
        energyLine1.setOrigin({
            energyLine1.getSize().x / 2.f,
            energyLine1.getSize().y / 2.f
        });
        energyLine1.setPosition({
            position.x + size.x / 2.f,
            position.y + size.y * 0.38f
        });
        energyLine1.setRotation(sf::degrees(-12.f));
        energyLine1.setFillColor(sf::Color(220, 245, 255, lineAlpha));

        sf::RectangleShape energyLine2;
        energyLine2.setSize({ size.x * 0.56f, 1.5f });
        energyLine2.setOrigin({
            energyLine2.getSize().x / 2.f,
            energyLine2.getSize().y / 2.f
        });
        energyLine2.setPosition({
            position.x + size.x / 2.f,
            position.y + size.y * 0.64f
        });
        energyLine2.setRotation(sf::degrees(14.f));
        energyLine2.setFillColor(sf::Color(220, 245, 255, static_cast<std::uint8_t>(lineAlpha * 0.8f)));

        sf::CircleShape spark1(1.8f);
        spark1.setOrigin({ 1.8f, 1.8f });
        spark1.setPosition({
            position.x + size.x * 0.28f,
            position.y + size.y * 0.32f
        });
        spark1.setFillColor(sf::Color(240, 255, 255, static_cast<std::uint8_t>(110.f * energyPulse)));

        sf::CircleShape spark2(1.8f);
        spark2.setOrigin({ 1.8f, 1.8f });
        spark2.setPosition({
            position.x + size.x * 0.72f,
            position.y + size.y * 0.68f
        });
        spark2.setFillColor(sf::Color(240, 255, 255, static_cast<std::uint8_t>(90.f * energyPulse)));

        window.draw(veil);
        window.draw(inner);
        window.draw(energyCore);
        window.draw(energyLine1);
        window.draw(energyLine2);
        window.draw(spark1);
        window.draw(spark2);

        if (ghostFlashTimer > 0.f)
        {
            const float flashRatio = ghostFlashTimer / 0.22f;

            const float expansion = 1.f + (1.f - flashRatio) * 0.18f;

            const std::uint8_t flashAlpha =
                static_cast<std::uint8_t>(110.f * flashRatio);

            sf::RectangleShape flashFrame;
            flashFrame.setSize({
                visualShape.getSize().x * expansion,
                visualShape.getSize().y * expansion
            });

            flashFrame.setOrigin({
                flashFrame.getSize().x / 2.f,
                flashFrame.getSize().y / 2.f
            });

            flashFrame.setPosition({
                visualShape.getPosition().x + visualShape.getSize().x / 2.f,
                visualShape.getPosition().y + visualShape.getSize().y / 2.f
            });

            flashFrame.setFillColor(sf::Color::Transparent);
            flashFrame.setOutlineThickness(2.f);
            flashFrame.setOutlineColor(sf::Color(220, 245, 255, flashAlpha));

            sf::CircleShape flashCore(visualShape.getSize().y * 0.16f * expansion);
            flashCore.setOrigin({
                visualShape.getSize().y * 0.16f * expansion,
                visualShape.getSize().y * 0.16f * expansion
            });
            flashCore.setPosition({
                visualShape.getPosition().x + visualShape.getSize().x / 2.f,
                visualShape.getPosition().y + visualShape.getSize().y / 2.f
            });
            flashCore.setScale({ 1.7f, 0.75f });
            flashCore.setFillColor(sf::Color(240, 255, 255,
                static_cast<std::uint8_t>(80.f * flashRatio)));

            window.draw(flashFrame);
            window.draw(flashCore);
        }
    }

    // Draw frost details on ice blocks.
    if (isIce())
    {
        const sf::Vector2f position = visualShape.getPosition();
        const sf::Vector2f size = visualShape.getSize();

        sf::RectangleShape topFrost;
        topFrost.setSize({ size.x - 6.f, 2.f });
        topFrost.setPosition({
            position.x + 3.f,
            position.y + 3.f
        });
        topFrost.setFillColor(sf::Color(235, 255, 255, 90));

        sf::RectangleShape innerFrost;
        innerFrost.setSize({ size.x - 10.f, size.y - 10.f });
        innerFrost.setPosition({
            position.x + 5.f,
            position.y + 5.f
        });
        innerFrost.setFillColor(sf::Color(210, 245, 255, 32));

        sf::CircleShape crystal(size.y * 0.16f);
        crystal.setOrigin({ size.y * 0.16f, size.y * 0.16f });
        crystal.setPosition({
            position.x + size.x / 2.f,
            position.y + size.y / 2.f
        });
        crystal.setScale({ 1.5f, 0.85f });
        crystal.setFillColor(sf::Color(235, 255, 255, 95));

        sf::RectangleShape iceLine1;
        iceLine1.setSize({ size.x * 0.72f, 1.5f });
        iceLine1.setOrigin({ 
            iceLine1.getSize().x / 2.f, 
            iceLine1.getSize().y / 2.f 
        });
        iceLine1.setPosition({
            position.x + size.x / 2.f,
            position.y + size.y * 0.36f
        });
        iceLine1.setRotation(sf::degrees(-18.f));
        iceLine1.setFillColor(sf::Color(240, 255, 255, 105));

        sf::RectangleShape iceLine2;
        iceLine2.setSize({ size.x * 0.58f, 1.5f });
        iceLine2.setOrigin({ 
            iceLine2.getSize().x / 2.f, 
            iceLine2.getSize().y / 2.f 
        });
        iceLine2.setPosition({
            position.x + size.x / 2.f,
            position.y + size.y * 0.64f
        });
        iceLine2.setRotation(sf::degrees(17.f));
        iceLine2.setFillColor(sf::Color(235, 255, 255, 85));

        sf::CircleShape shard1(2.f);
        shard1.setOrigin({ 2.f, 2.f });
        shard1.setPosition({
            position.x + size.x * 0.22f,
            position.y + size.y * 0.30f
        });
        shard1.setFillColor(sf::Color(240, 255, 255, 120));

        sf::CircleShape shard2(2.f);
        shard2.setOrigin({ 2.f, 2.f });
        shard2.setPosition({
            position.x + size.x * 0.76f,
            position.y + size.y * 0.68f
        });
        shard2.setFillColor(sf::Color(240, 255, 255, 95));

        window.draw(innerFrost);
        window.draw(topFrost);
        window.draw(crystal);
        window.draw(iceLine1);
        window.draw(iceLine2);
        window.draw(shard1);
        window.draw(shard2);
    }

    // Draw animated portal rings and teleport flash.
    if (isPortal())
    {
        const float pulse =
            1.f + 0.08f * std::sin(portalAnimationTimer * 3.2f + portalPhase);

        const float ringRadius =
            std::min(visualShape.getSize().x, visualShape.getSize().y) * 0.26f * pulse;

        const sf::Color primaryColor = getPortalPrimaryColor();
        const sf::Color secondaryColor = getPortalSecondaryColor();

        const sf::Vector2f portalCenter{
            visualShape.getPosition().x + visualShape.getSize().x / 2.f,
            visualShape.getPosition().y + visualShape.getSize().y / 2.f
        };

        sf::CircleShape outerRing(ringRadius);
        outerRing.setOrigin({ ringRadius, ringRadius });
        outerRing.setPosition(portalCenter);
        outerRing.setScale({ 1.4f, 0.75f });
        outerRing.setFillColor(sf::Color::Transparent);
        outerRing.setOutlineThickness(2.f);
        outerRing.setOutlineColor(sf::Color(
            primaryColor.r,
            primaryColor.g,
            primaryColor.b,
            210
        ));

        sf::CircleShape innerRing(ringRadius * 0.62f);
        innerRing.setOrigin({ ringRadius * 0.62f, ringRadius * 0.62f });
        innerRing.setPosition(portalCenter);
        innerRing.setScale({ 1.2f, 0.60f });
        innerRing.setFillColor(sf::Color::Transparent);
        innerRing.setOutlineThickness(2.f);
        innerRing.setOutlineColor(sf::Color(
            secondaryColor.r,
            secondaryColor.g,
            secondaryColor.b,
            180
        ));

        sf::CircleShape core(ringRadius * 0.24f);
        core.setOrigin({ ringRadius * 0.24f, ringRadius * 0.24f });
        core.setPosition(portalCenter);
        core.setScale({ 1.1f, 0.65f });
        core.setFillColor(sf::Color(
            secondaryColor.r,
            secondaryColor.g,
            secondaryColor.b,
            90
        ));

        window.draw(outerRing);
        window.draw(innerRing);
        window.draw(core);

        if (portalFlashTimer > 0.f)
        {
            const float flashRatio = portalFlashTimer / 0.28f;

            const float flashRadius =
                ringRadius * (1.4f + (1.f - flashRatio) * 1.5f);

            const std::uint8_t flashAlpha =
                static_cast<std::uint8_t>(180.f * flashRatio);

            sf::CircleShape flashRing(flashRadius);
            flashRing.setOrigin({ flashRadius, flashRadius });
            flashRing.setPosition(portalCenter);
            flashRing.setScale({ 1.45f, 0.78f });
            flashRing.setFillColor(sf::Color::Transparent);
            flashRing.setOutlineThickness(3.f);
            flashRing.setOutlineColor(sf::Color(
                secondaryColor.r,
                secondaryColor.g,
                secondaryColor.b,
                flashAlpha
            ));

            window.draw(flashRing);
        }
    }

    // Add a simple top highlight only to solid non-special blocks.
    if (!isDamagedMetal() && !isPortal() && !isIce() && !isGhostBlock())
    {
        sf::RectangleShape highlight;
        highlight.setSize({ visualShape.getSize().x, 1.f });
        highlight.setPosition(visualShape.getPosition());
        highlight.setFillColor(sf::Color(255, 255, 255, 25));

        window.draw(highlight);
    }
}

// Applies normal block damage and updates strong block state.
void Brick::hit()
{
    // Permanent interactive blocks ignore normal damage.
    if (isMetal() || isBumper() || isPortal() || isGhostBlock())
        return;

    if (destroyed)
        return;

    --hitPoints;

    if (hitPoints <= 0)
    {
        destroyed = true;
        return;
    }

    // Strong blocks change appearance after the first hit.
    if (hitPoints == 1 && type == 3)
    {
        type = 4;
        shape.setFillColor(sf::Color(255, 110, 110));
    }
}

sf::FloatRect Brick::getBounds() const
{
    return shape.getGlobalBounds();
}

bool Brick::isDestroyed() const
{
    return destroyed || hitPoints <= 0;
}

int Brick::getType() const
{
    return type;
}

bool Brick::isMetal() const
{
    return type == 5;
}

bool Brick::isMoving() const
{
    return moving;
}

float Brick::getMoveDirection() const
{
    return moveDirection;
}

float Brick::getMoveSpeed() const
{
    return moveSpeed;
}

void Brick::setMoveDirection(float direction)
{
    moveDirection = direction;
}

void Brick::move(sf::Vector2f offset)
{
    shape.move(offset);
}

// Updates temporary hit feedback for metal blocks.
void Brick::updateHitEffect(float deltaTime)
{
    if (hitEffectTimer > 0.f)
    {
        hitEffectTimer -= deltaTime;

        if (hitEffectTimer < 0.f)
            hitEffectTimer = 0.f;
    }
}

// Starts a short visual feedback effect after hitting metal.
void Brick::triggerMetalHitEffect()
{
    if (!isMetal())
        return;

    hitEffectTimer = 0.16f;
}

bool Brick::isBumper() const
{
    return type == 6;
}

// Moves bumper blocks vertically within their allowed range.
void Brick::updateBumper(float deltaTime)
{
    if (!isBumper() || destroyed)
        return;

    shape.move({
        0.f,
        bumperDirection * Constants::BumperMoveSpeed * deltaTime
     });

    const float currentY = shape.getPosition().y;

    if (currentY > bumperStartY + Constants::BumperMoveRange)
    {
        shape.setPosition({
            shape.getPosition().x,
            bumperStartY + Constants::BumperMoveRange
        });

        bumperDirection = -1.f;
    }
    else if (currentY < bumperStartY - Constants::BumperMoveRange)
    {
        shape.setPosition({
            shape.getPosition().x,
            bumperStartY - Constants::BumperMoveRange
        });

        bumperDirection = 1.f;
    }

    if (bumperHitEffectTimer > 0.f)
    {
        bumperHitEffectTimer -= deltaTime;

        if (bumperHitEffectTimer < 0.f)
            bumperHitEffectTimer = 0.f;
    }
}

// Starts a short pulse when the ball hits a bumper.
void Brick::triggerBumperHitEffect()
{
    if (!isBumper())
        return;

    bumperHitEffectTimer = 0.18f;
}

// Forces the block into a destroyed state.
void Brick::destroy()
{
    destroyed = true;
    hitPoints = 0;
}

bool Brick::isDamagedMetal() const
{
    return isMetal() && metalDamageLevel > 0;
}

// Fireballs disable moving metal first, then destroy it on the next fire hit.
void Brick::hitByFireball()
{
    if (!isMetal())
        return;

    if (metalDamageLevel == 0)
    {
        metalDamageLevel = 1;

        moving = false;
        moveSpeed = 0.f;

        shape.setFillColor(sf::Color(95, 105, 120));

        return;
    }

    destroy();
}

bool Brick::isPortal() const
{
    return type >= 7 && type <= 9;
}

int Brick::getPortalGroup() const
{
    if (!isPortal())
        return -1;

    return type;
}

// Returns the main portal color used for rendering and particles.
sf::Color Brick::getPortalPrimaryColor() const
{
    switch (type)
    {
    case 7:
        return sf::Color(155, 95, 255);    
    case 8:
        return sf::Color(90, 220, 255);    
    case 9:
        return sf::Color(255, 100, 185);    
    default:
        return sf::Color(150, 90, 255);
    }
}

// Returns the secondary portal color used for rendering and particles.
sf::Color Brick::getPortalSecondaryColor() const
{
    switch (type)
    {
    case 7:
        return sf::Color(220, 180, 255);
    case 8:
        return sf::Color(180, 245, 255);
    case 9:
        return sf::Color(255, 185, 225);
    default:
        return sf::Color(220, 180, 255);
    }
}

// Updates shared animation timing for portals and ghost blocks.
void Brick::updatePortalAnimation(float deltaTime)
{
    if ((!isPortal() && !isGhostBlock()) || destroyed)
        return;

    portalAnimationTimer += deltaTime;
}

// Starts a teleport flash on portal entry or exit.
void Brick::triggerPortalFlash()
{
    if (!isPortal())
        return;

    portalFlashTimer = 0.28f;
}

void Brick::updatePortalFlash(float deltaTime)
{
    if (portalFlashTimer <= 0.f)
        return;

    portalFlashTimer -= deltaTime;

    if (portalFlashTimer < 0.f)
        portalFlashTimer = 0.f;
}

bool Brick::isIce() const
{
    return type == 10;
}

bool Brick::isGenerator() const
{
    return type == 11;
}

bool Brick::isGhostBlock() const
{
    return type == 12;
}

int Brick::getLinkGroup() const
{
    return linkGroup;
}

// Starts a short flash when a ball passes through a ghost block.
void Brick::triggerGhostFlash()
{
    if (!isGhostBlock())
        return;

    ghostFlashTimer = 0.22f;
}

void Brick::updateGhostFlash(float deltaTime)
{
    if (ghostFlashTimer <= 0.f)
        return;

    ghostFlashTimer -= deltaTime;

    if (ghostFlashTimer < 0.f)
        ghostFlashTimer = 0.f;
}