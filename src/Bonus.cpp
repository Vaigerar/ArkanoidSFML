#include "Bonus.h"
#include "Constants.h"

// Initializes bonus shape, color, and starting position.
Bonus::Bonus(sf::Vector2f position, BonusType bonusType)
	: type(bonusType)
{
	shape.setSize({ 28.f, 18.f });
	shape.setOrigin({ 14.f, 9.f });
	shape.setPosition(position);

	// Use color to make each bonus type readable in-game.
	switch (type)
	{
	case BonusType::ExpandPaddle:
		shape.setFillColor(sf::Color(255, 230, 80));
		break;

	case BonusType::ExtraBall:
		shape.setFillColor(sf::Color(180, 120, 255));
		break;
	}
}

// Moves the bonus downward over time.
void Bonus::update(float deltaTime)
{
	shape.move({ 0.f, fallSpeed * deltaTime });
}

// Draws the bonus icon and optional text label.
void Bonus::draw(sf::RenderWindow& window, const sf::Font& font, bool fontLoaded) const
{
	sf::RectangleShape shadow = shape;
	shadow.move({ 3.f, 3.f });
	shadow.setFillColor(sf::Color(0, 0, 0, 80));

	sf::RectangleShape glow;
	glow.setSize({
		shape.getSize().x + 10.f,
		shape.getSize().y + 10.f
	});
	glow.setPosition({
		shape.getPosition().x - 5.f,
		shape.getPosition().y - 5.f
	});

	if (type == BonusType::ExpandPaddle)
		glow.setFillColor(sf::Color(255, 230, 80, 50));
	else
		glow.setFillColor(sf::Color(180, 120, 255, 50));

	window.draw(glow);
	window.draw(shadow);
	window.draw(shape);

	if (!fontLoaded)
		return;

	const sf::String label = type == BonusType::ExpandPaddle ? "E" : "B";

	sf::Text shadowText(font, label, 14);
	shadowText.setFillColor(sf::Color(0, 0, 0, 180));
	shadowText.setStyle(sf::Text::Bold);

	const sf::FloatRect shadowBounds = shadowText.getLocalBounds();

	shadowText.setOrigin({
		shadowBounds.position.x + shadowBounds.size.x / 2.f,
		shadowBounds.position.y + shadowBounds.size.y / 2.f
	});

	shadowText.setPosition({
		shape.getPosition().x + 1.f,
		shape.getPosition().y + 1.f
	});

	sf::Text text(font, label, 14);
	text.setFillColor(sf::Color(245, 250, 255));
	text.setStyle(sf::Text::Bold);

	const sf::FloatRect textBounds = text.getLocalBounds();

	text.setOrigin({
		textBounds.position.x + textBounds.size.x / 2.f,
		textBounds.position.y + textBounds.size.y / 2.f
	});

	text.setPosition(shape.getPosition());

	window.draw(shadowText);
	window.draw(text);
}

sf::FloatRect Bonus::getBounds() const
{
	return shape.getGlobalBounds();
}

BonusType Bonus::getType() const
{
	return type;
}

bool Bonus::isOutOfScreen() const
{
	return shape.getPosition().y > Constants::WindowHeight + 30.f;
}