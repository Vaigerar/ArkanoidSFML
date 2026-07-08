#pragma once

#include <SFML/Graphics.hpp>

// Types of bonuses that can fall from bonus blocks.
enum class BonusType
{
	ExpandPaddle,
	ExtraBall
};

class Bonus
{
public:
	// Creates a falling bonus at the given position.
	Bonus(sf::Vector2f position, BonusType type);

	// Movement, rendering, and collision methods.
	void update(float deltaTime);
	void draw(sf::RenderWindow& window, const sf::Font& font, bool fontLoaded) const;

	sf::FloatRect getBounds() const;
	BonusType getType() const;

	bool isOutOfScreen() const;

private:
	// Bonus visual and collision shape.
	sf::RectangleShape shape;
	
	// Stored bonus effect type.
	BonusType type;

	// Downward movement speed.
	float fallSpeed = 260.f;
};