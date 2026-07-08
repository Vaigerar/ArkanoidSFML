#include "Paddle.h"
#include "Constants.h"
#include <algorithm>

// Initializes paddle size, origin, color, and starting position.
Paddle::Paddle()
{
    shape.setSize({Constants::PaddleWidth, Constants::PaddleHeight});
    shape.setFillColor(sf::Color::White);
    reset();
}

// Restores the paddle to its default size and starting position.
void Paddle::reset()
{
    const float startX = Constants::WindowWidth / 2.f - Constants::PaddleWidth / 2.f;
    shape.setSize({ Constants::PaddleWidth, Constants::PaddleHeight });
    shape.setPosition({ startX, Constants::PaddleY });
}

// Moves the paddle while keeping it inside the screen bounds.
void Paddle::update(float deltaTime, bool moveLeft, bool moveRight)
{
    float dx = 0.f;

    if (moveLeft)
        dx -= Constants::PaddleSpeed * deltaTime;

    if (moveRight)
        dx += Constants::PaddleSpeed * deltaTime;

    shape.move({dx, 0.f});

    const float clampedX = std::clamp(
        shape.getPosition().x,
        0.f,
        static_cast<float>(Constants::WindowWidth) - Constants::PaddleWidth
    );

    shape.setPosition({clampedX, Constants::PaddleY});
}

// Draws the paddle with a simple shadow and glow effect.
void Paddle::draw(sf::RenderWindow& window) const
{
    sf::RectangleShape shadow = shape;
    shadow.move({ Constants::ShadowOffsetX, Constants::ShadowOffsetY });
    shadow.setFillColor(sf::Color(0, 0, 0, 90));

    window.draw(shadow);
    window.draw(shape);

    sf::RectangleShape highlight;
    highlight.setSize({ shape.getSize().x, 3.f });
    highlight.setPosition(shape.getPosition());
    highlight.setFillColor(sf::Color(255, 255, 255, 70));

    window.draw(highlight);
}

sf::FloatRect Paddle::getBounds() const
{
    return shape.getGlobalBounds();
}

sf::Vector2f Paddle::getPosition() const
{
    return shape.getPosition();
}

float Paddle::getCenterX() const
{
    return shape.getPosition().x + Constants::PaddleWidth / 2.f;
}

// Changes paddle width when the expand bonus is active.
void Paddle::setWidth(float width)
{
    shape.setSize({ width, Constants::PaddleHeight });
}