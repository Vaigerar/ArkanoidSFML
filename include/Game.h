#pragma once

#include "Ball.h"
#include "Brick.h"
#include "Paddle.h"
#include "Bonus.h"
#include "Constants.h"
#include <SFML/Graphics.hpp>
#include <vector>
#include <cstddef>

class Game
{
public:
    // Main game lifecycle methods.
    Game();

    void run();

private:
    // Main game flow states.
    enum class State
    {
        MainMenu,
        Playing,
        Paused,
        GameOver,
        Win
    };

    // Lightweight particle used for impact and special effects.
    struct Spark
    {
        sf::Vector2f position;
        sf::Vector2f velocity;
        sf::Color color = sf::Color(220, 235, 255, 180);
        float lifetime = 0.f;
        float maxLifetime = 0.28f;
        float radius = 2.f;
    };

    // Fire clouds grant fireball status when touched by a ball.
    struct FireCloud
    {
        sf::Vector2f position;
        float radius = Constants::FireCloudRadius;
        float pulsePhase = 0.f;
        float flickerPhase = 0.f;
    };

    // Main update pipeline.
    void processEvents();
    void update(float deltaTime);
    void render();

    // Level loading and game reset helpers.
    void loadLevel(int levelNumber);
    void resetBallAndPaddle();
    void restartGame();

    // Collision handling for the main ball, extra balls, paddle, and bricks.
    void handleMainBallWallCollisions();
    bool handleExtraBallWallCollisions(std::size_t extraBallIndex);
    void handlePaddleCollision(Ball& targetBall);
    void handleBrickCollisions(Ball& targetBall);
    
    // Win condition check.
    bool areAllBricksDestroyed() const;

    // Background and UI rendering helpers.
    void drawBackground();
    void drawInterface();
    void drawCenteredMessage(const sf::String& message, float y);

    // Bonus spawning, collection, and lifetime handling.
    void spawnBonusFromBrick(const Brick& brick);
    void updateBonuses(float deltaTime);
    void applyBonus(BonusType type);
    void updatePaddleBonusTimer(float deltaTime);
    void updateExtraBallTimer(float deltaTime);

    // Particle effect helpers.
    void spawnImpactSparks(sf::Vector2f position);
    void spawnFireImpactSparks(sf::Vector2f position);
    void spawnPortalSparks(sf::Vector2f position, sf::Color primaryColor, sf::Color secondaryColor);
    void updateSparks(float deltaTime);
    void drawSparks();

    // Fire cloud spawning, rendering, and collision logic.
    void updateFireClouds(float deltaTime);
    void drawFireClouds();
    void trySpawnFireCloud();

    bool isValidFireCloudPosition(sf::Vector2f position) const;
    void checkFireCloudCollision(Ball& targetBall);

    // Portal teleportation mechanics.
    void teleportBallThroughPortal(Ball& targetBall, Brick& entryPortal);

    // Generator and ghost block mechanics.
    void handleGhostBlockCollision(Ball& targetBall, Brick& ghostBlock);
    void explodeGenerator(Brick& generatorBrick);

    // Moving metal block logic.
    void updateMovingBricks(float deltaTime);

    bool isBrickMovementBlocked(
        std::size_t movingBrickIndex,
        float offsetX,
        std::size_t& blockingBrickIndex
    ) const;

private:
    // Window, timing, and current game state.
    sf::RenderWindow window;
    sf::Font font;
    State state = State::MainMenu;
    
    bool fontLoaded = false;

    // Core gameplay objects.
    Ball ball;
    Paddle paddle;
    std::vector<Brick> bricks;
    std::vector<Bonus> bonuses;
    std::vector<Ball> extraBalls;
    
    // Active particle effects.
    std::vector<Spark> sparks;

    // Active fire clouds and their spawn timing.
    std::vector<FireCloud> fireClouds;

    float fireCloudSpawnTimer = 0.f;
    float fireCloudVisualTimer = 0.f;
    
    int bonusSpawnCounter = 0;

    // Active bonus timers.
    float paddleBonusTimer = 0.f;
    int paddleBonusLevel = 0;
    float extraBallTimer = 0.f;

    // Player progress and score state.
    int lives = 3;
    int score = 0;
    int currentLevel = 1;

    // Current input state.
    bool moveLeft = false;
    bool moveRight = false;
};
