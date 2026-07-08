#pragma once

#include <SFML/Graphics.hpp>

class Paddle
{
public:
    // Draws centered menu, pause, win, and game over messages.
    Paddle();

    // Movement, rendering, and reset methods.
    void update(float deltaTime, bool moveLeft, bool moveRight);
    void draw(sf::RenderWindow& window) const;
    void reset();

    // Position and size helpers used by collision and bonus logic.
    sf::FloatRect getBounds() const;
    sf::Vector2f getPosition() const;
    float getCenterX() const;

    void setWidth(float width);

private:
    // Paddle collision and rendering shape.
    sf::RectangleShape shape;
};
