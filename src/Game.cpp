#include "Game.h"
#include "Constants.h"
#include <cstddef>
#include <array>
#include <string>
#include <cstdint>
#include <cmath>
#include <algorithm>
#include <random>
#include <iostream>

namespace
{
    // Level legend:
    // 0 = empty cell
    // 1 = normal block
    // 2 = bonus block
    // 3 = strong block
    // 4 = damaged strong block, usually not placed manually
    // 5 = moving metal block
    // 6 = bumper block
    // 7/8/9 = portal groups
    // A = ice block
    // B/C/D = generator blocks
    // b/c/d = ghost blocks linked to B/C/D
    const std::array<std::string, Constants::LevelRows> Level1 =
    {
        "00000000000000000000000000000000",
        "00000000000000000000000000000000",

        "221100005A7A00111110A8A500001122",
        "1122A1122A1122AABAA2211A2211A221",
        "11D11AA223112211122111322AA11C11",
        "A222200111330005500033111002222A",
        "9112200AA21110033001112AA0022119",
        "2112100AA21110022001112AA0012112",
        "0533100AA21110033001112AA0013350",
        "01111223311211155111211332211110",
        "02300000AA7AAd3223dAA8AA00000320",
        "0230000000511A1111A1150000000320",

        "001200002b211112211112b200000210",
        "6012002c2111112222111112c2000216",
        "00000000000001133110000000000000",
        "00000000000000000000000000000000",
        "00000000000000000000000000000000",
        "00000000000000000000000000000000"
    };
}

// Creates the main window, loads resources, and prepares the first game state.
Game::Game()
    : window(
        sf::VideoMode({
            static_cast<unsigned int>(Constants::WindowWidth), 
            static_cast<unsigned int>(Constants::WindowHeight)
        }), 
        "Arkanoid SFML",
        sf::State::Fullscreen
      )
{
    window.setFramerateLimit(60);

    // Optional. Put arial.ttf or another font near the executable to display text.
    // The game itself works even if the font is not found.
    fontLoaded = font.openFromFile("C:/Windows/Fonts/arial.ttf");

    loadLevel(currentLevel);
    resetBallAndPaddle();
    state = State::MainMenu;
}

// Main game loop.
void Game::run()
{
    sf::Clock clock;

    while (window.isOpen())
    {
        // Measure elapsed time to keep movement frame-rate independent.
        const float deltaTime = clock.restart().asSeconds();

        processEvents();

        if (state == State::Playing)
            update(deltaTime);

        render();
    }
}

// Handles window events and player input.
void Game::processEvents()
{
    while (const auto event = window.pollEvent())
    {
        // Close the game window when the user requests it.
        if (event->is<sf::Event::Closed>())
        {
            window.close();
            continue;
        }

        // Handle one-time key press actions.
        if (const auto* keyPressed = event->getIf<sf::Event::KeyPressed>())
        {
            const sf::Keyboard::Key key = keyPressed->code;

            if (key == sf::Keyboard::Key::Escape)
                window.close();

            if (key == sf::Keyboard::Key::Enter && state == State::MainMenu)
            {
                restartGame();
                continue;
            }

            if (key == sf::Keyboard::Key::Left || key == sf::Keyboard::Key::A)
                moveLeft = true;

            if (key == sf::Keyboard::Key::Right || key == sf::Keyboard::Key::D)
                moveRight = true;

            if (key == sf::Keyboard::Key::Space && state == State::Playing)
                state = State::Paused;
            else if (key == sf::Keyboard::Key::Space && state == State::Paused)
                state = State::Playing;

            if (key == sf::Keyboard::Key::Enter &&
                (state == State::GameOver || state == State::Win))
            {
                restartGame();
                continue;
            }
                
        }

        // Stop continuous movement when movement keys are released.
        if (const auto* keyReleased = event->getIf<sf::Event::KeyReleased>())
        {
            const sf::Keyboard::Key key = keyReleased->code;

            if (key == sf::Keyboard::Key::Left || key == sf::Keyboard::Key::A)
                moveLeft = false;

            if (key == sf::Keyboard::Key::Right || key == sf::Keyboard::Key::D)
                moveRight = false;
        }
    }
}

// Updates gameplay logic while the game is in the Playing state.
void Game::update(float deltaTime)
{
    // Do not update gameplay while in menu, pause, win, or game over states.
    if (state != State::Playing)
        return;

    paddle.update(deltaTime, moveLeft, moveRight);
    
    updateMovingBricks(deltaTime);

    // Update block animations and temporary visual effects.
    for (Brick& brick : bricks) 
    {
        brick.updateHitEffect(deltaTime);
        brick.updateBumper(deltaTime);
        brick.updatePortalAnimation(deltaTime);
        brick.updatePortalFlash(deltaTime);
        brick.updateGhostFlash(deltaTime);
    }
    
    // Update the main ball and all active temporary effects.
    ball.update(deltaTime);
    ball.updateSpeedBoost(deltaTime);
    ball.updateFireball(deltaTime);
    ball.updateSlow(deltaTime);
    ball.updateTeleportCooldown(deltaTime);
    ball.updateTrajectoryShift(deltaTime);

    // Update extra balls using the same effect logic as the main ball.
    for (Ball& extraBall : extraBalls)
    {
        extraBall.update(deltaTime);
        extraBall.updateSpeedBoost(deltaTime);
        extraBall.updateFireball(deltaTime);
        extraBall.updateSlow(deltaTime);
        extraBall.updateTeleportCooldown(deltaTime);
        extraBall.updateTrajectoryShift(deltaTime);
    }

    updateFireClouds(deltaTime);

    checkFireCloudCollision(ball);

    for (Ball& extraBall : extraBalls)
        checkFireCloudCollision(extraBall);

    handleMainBallWallCollisions();
    handlePaddleCollision(ball);
    handleBrickCollisions(ball);

    // Use index-based iteration because extra balls can be removed during collision handling.
    std::size_t extraBallIndex = 0;

    while (extraBallIndex < extraBalls.size())
    {
        if (handleExtraBallWallCollisions(extraBallIndex))
            continue;

        handlePaddleCollision(extraBalls[extraBallIndex]);
        handleBrickCollisions(extraBalls[extraBallIndex]);

        ++extraBallIndex;
    }

    updateBonuses(deltaTime);
    updateSparks(deltaTime);
    updateExtraBallTimer(deltaTime);
    updatePaddleBonusTimer(deltaTime);

    // Win when all destructible blocks are gone.
    if (areAllBricksDestroyed())
        state = State::Win;
}

// Draws the current frame.
void Game::render()
{
    window.clear(sf::Color(10, 12, 22));

    drawBackground();
    drawFireClouds();

    // Draw all active blocks.
    for (const Brick& brick : bricks)
        brick.draw(window);

    drawSparks();

    // Draw falling bonuses.
    for (const Bonus& bonus : bonuses)
        bonus.draw(window, font, fontLoaded);

    paddle.draw(window);
    ball.draw(window);

    // Draw extra balls after the main ball.
    for (const Ball& extraBall : extraBalls)
        extraBall.draw(window);

    drawInterface();

    if (state == State::MainMenu)
        drawCenteredMessage("ARKANOID\n\nPress Enter to start\nEsc to exit", Constants::WindowHeight / 2.f - 60.f);
    else if (state == State::Paused)
        drawCenteredMessage("Pause", Constants::WindowHeight / 2.f - 20.f);
    else if (state == State::GameOver)
        drawCenteredMessage("GAME OVER\nPress Enter to restart", Constants::WindowHeight / 2.f - 40.f);
    else if (state == State::Win)
        drawCenteredMessage("LEVEL COMPLETE\nPress Enter to restart", Constants::WindowHeight / 2.f - 40.f);

    window.display();
}

// Loads a level from the character map and converts symbols into brick objects.
void Game::loadLevel(int levelNumber)
{
    (void)levelNumber;

    // Clear previous level data before creating the new layout.
    bricks.clear();

    const float levelWidth = 
        static_cast<float>(Constants::LevelColumns) * Constants::BrickWidth;

    const float startX = 
        (static_cast<float>(Constants::WindowWidth) - levelWidth) / 2.f;

    const float startY = 140.f;

    for (int row = 0; row < Constants::LevelRows; ++row)
    {
        for (int column = 0; column < Constants::LevelColumns; ++column)
        {
            // Read the level symbol at the current grid cell.
            const char cell = Level1[row][column];

            if (cell == '0')
                continue;

            int brickType = 0;
            
            // Link groups connect generators with their matching ghost blocks.
            int linkGroup = -1;

            if (cell >= '1' && cell <= '9')
            {
                brickType = cell - '0';
            }
            else if (cell == 'A')
            {
                brickType = 10; 
            }
            else if (cell == 'B')
            {
                brickType = 11; 
                linkGroup = 0;
            }
            else if (cell == 'b')
            {
                brickType = 12; 
                linkGroup = 0;
            }
            else if (cell == 'C')
            {
                brickType = 11; 
                linkGroup = 1;
            }
            else if (cell == 'c')
            {
                brickType = 12; 
                linkGroup = 1;
            }
            else if (cell == 'D')
            {
                brickType = 11; 
                linkGroup = 2;
            }
            else if (cell == 'd')
            {
                brickType = 12; 
                linkGroup = 2;
            }
            else
            {
                continue;
            }

            const float x =
                startX + static_cast<float>(column) * Constants::BrickWidth;

            const float y = 
                startY + static_cast<float>(row) * Constants::BrickHeight;

            // Create the final brick using its type and optional link group.
            bricks.emplace_back(
                sf::Vector2f(x, y),
                brickType,
                linkGroup
            );
        }
    }
}

// Resets the paddle and main ball after losing a life.
void Game::resetBallAndPaddle()
{
    ball.reset();
    paddle.reset();
    moveLeft = false;
    moveRight = false;
}

// Restarts the full game state from the first level.
void Game::restartGame()
{
    currentLevel = 1;
    lives = Constants::StartLives;
    score = 0;

    bonusSpawnCounter = 0;
    bonuses.clear();

    fireClouds.clear();
    fireCloudSpawnTimer = 0.f;
    fireCloudVisualTimer = 0.f;

    paddleBonusLevel = 0;
    paddleBonusTimer = 0.f;

    extraBalls.clear();
    extraBallTimer = 0.f;

    state = State::Playing;

    loadLevel(currentLevel);
    resetBallAndPaddle();
}

// Handles wall collisions for the main ball, including life loss.
void Game::handleMainBallWallCollisions()
{
    const sf::FloatRect bounds = ball.getBounds();

    if (bounds.position.x <= 0.f)
        ball.reverseX();

    if (bounds.position.x + bounds.size.x >= Constants::WindowWidth)
        ball.reverseX();

    if (bounds.position.y <= 0.f)
        ball.reverseY();

    // Only the main ball can fall below the screen and reduce lives.
    if (bounds.position.y + bounds.size.y >= Constants::WindowHeight)
    {
        --lives;

        if (lives <= 0)
        {
            state = State::GameOver;
            return;
        }

        resetBallAndPaddle();
    }
}

// Removes extra balls when they fall without reducing player lives.
bool Game::handleExtraBallWallCollisions(std::size_t extraBallIndex)
{
    if (extraBallIndex >= extraBalls.size())
        return true;

    Ball& extraBall = extraBalls[extraBallIndex];

    const sf::FloatRect bounds = extraBall.getBounds();

    if (bounds.position.x <= 0.f)
        extraBall.reverseX();

    if (bounds.position.x + bounds.size.x >= Constants::WindowWidth)
        extraBall.reverseX();

    if (bounds.position.y <= 0.f)
        extraBall.reverseY();

    if (bounds.position.y + bounds.size.y >= Constants::WindowHeight)
    {
        extraBalls.erase(
            extraBalls.begin() + static_cast<std::ptrdiff_t>(extraBallIndex)
        );

        if (extraBalls.empty())
            extraBallTimer = 0.f;

        return true;
    }

    return false;
}

// Bounces a ball from the paddle and adjusts its horizontal direction.
void Game::handlePaddleCollision(Ball& targetBall)
{
    if (targetBall.getBounds().findIntersection(paddle.getBounds()).has_value())
    {

        sf::Vector2f ballPosition = targetBall.getPosition();
        sf::Vector2f velocity = targetBall.getVelocity();

        if (velocity.y <= 0.f)
            return;

        const float offset = ballPosition.x - paddle.getCenterX();
        
        const float paddleWidth = paddle.getBounds().size.x;
        const float normalizedOffset = offset / (paddleWidth / 2.f);

        velocity.x = normalizedOffset * 260.f;
        velocity.y = -std::abs(velocity.y);

        targetBall.setVelocity(velocity.x, velocity.y);
        targetBall.setPosition(
            ballPosition.x, 
            Constants::PaddleY - Constants::BallRadius - 1.f
        );

    }
}

// Handles block collisions and applies type-specific mechanics.
void Game::handleBrickCollisions(Ball& targetBall)
{
    for (Brick& brick : bricks)
    {
        if (brick.isDestroyed())
            continue;

        if (!targetBall.getBounds().findIntersection(brick.getBounds()).has_value())
            continue;

        // Ghost blocks are pass-through triggers, so they must be handled before bounce logic.
        if (brick.isGhostBlock())
        {
            handleGhostBlockCollision(targetBall, brick);
            return;
        }

        // Portals teleport the ball without bouncing or consuming fireball status.
        if (brick.isPortal())
        {
            teleportBallThroughPortal(targetBall, brick);
            return;
        }

        // Solid blocks bounce the ball before applying their special effects.
        const sf::FloatRect ballBounds = targetBall.getBounds();
        const sf::FloatRect brickBounds = brick.getBounds();

        const float overlapLeft = 
            ballBounds.position.x + ballBounds.size.x - brickBounds.position.x;
        
        const float overlapRight = 
            brickBounds.position.x + brickBounds.size.x - ballBounds.position.x;
        
        const float overlapTop = 
            ballBounds.position.y + ballBounds.size.y - brickBounds.position.y;
        
        const float overlapBottom = 
            brickBounds.position.y + brickBounds.size.y - ballBounds.position.y;

        const float minOverlapX = std::min(overlapLeft, overlapRight);
        const float minOverlapY = std::min(overlapTop, overlapBottom);

        if (minOverlapX < minOverlapY)
            targetBall.reverseX();
        else
            targetBall.reverseY();

        const int brickTypeBeforeHit = brick.getType();

        const sf::Vector2f sparkPosition{
            ballBounds.position.x + ballBounds.size.x / 2.f,
            ballBounds.position.y + ballBounds.size.y / 2.f
        };

        // Generator blocks explode and remove their linked ghost blocks.
        if (brick.isGenerator())
        {
            explodeGenerator(brick);

            if (targetBall.isFireball())
                targetBall.consumeFireball();

            return;
        }

        // Metal blocks ignore normal hits and can only be damaged by fireballs.
        if (brick.isMetal())
        {
            brick.triggerMetalHitEffect();

            if (targetBall.isFireball())
            {
                spawnFireImpactSparks(sparkPosition);
                brick.hitByFireball();
                targetBall.consumeFireball();
            }

            return;
        }

        // Bumpers cleanse slow, boost the ball, and consume fireball status.
        if (brick.isBumper())
        {
            brick.triggerBumperHitEffect();

            if (targetBall.isSlowed())
                targetBall.consumeSlow();

            targetBall.activateSpeedBoost(
                Constants::BallSpeedBoostMultiplier,
                Constants::BallSpeedBoostDuration
            );

            if (targetBall.isFireball())
            {
                spawnFireImpactSparks(sparkPosition);
                targetBall.consumeFireball();
            }
            else
            {
                spawnImpactSparks(sparkPosition);
            }

            return;
        }

        // Fireballs destroy destructible blocks without applying ice slow.
        if (targetBall.isFireball())
        {
            brick.destroy();
            targetBall.consumeFireball();

            score += 150;

            if (brickTypeBeforeHit == 2)
                spawnBonusFromBrick(brick);

            spawnFireImpactSparks(sparkPosition);

            return;
        }

        // Ice blocks slow the ball only when destroyed by a non-fireball hit.
        if (brick.isIce())
        {
            brick.hit();

            score += 120;

            targetBall.activateSlow(
                Constants::IceBlockSlowMultiplier,
                Constants::IceBlockSlowDuration
            );

            spawnImpactSparks(sparkPosition);

            return;
        }

        // Default damage handling for normal, bonus, and strong blocks.
        brick.hit();

        if (brick.isDestroyed())
        {
            score += 100;

            if (brickTypeBeforeHit == 2)
                spawnBonusFromBrick(brick);
        }
        else
        {
            score += 25;
        }

        return;
    }
}

// Checks whether all destructible blocks have been cleared.
bool Game::areAllBricksDestroyed() const
{
    for (const Brick& brick : bricks)
    {
        // Permanent interactive blocks do not count toward level completion.
        if (brick.isMetal() || brick.isBumper() || brick.isPortal())
            continue;
        
        if (!brick.isDestroyed())
            return false;
    }

    return true;
}

// Draws lives, score, and active status hints.
void Game::drawInterface()
{
    if (!fontLoaded || state == State::MainMenu)
        return;

    std::string info =
        "Lives: " + std::to_string(lives) +
        "   Score: " + std::to_string(score) +
        "   Space: Pause";

    if (ball.isFireball())
    {
        info += "   FIREBALL";
    }

    sf::Text shadow(font, info, 24);
    shadow.setFillColor(sf::Color(0, 0, 0, 170));
    shadow.setPosition({ 27.f, 27.f });

    sf::Text text(font, info, 24);
    text.setFillColor(sf::Color(220, 240, 255));
    text.setPosition({ 25.f, 25.f });

    window.draw(shadow);
    window.draw(text);
}

// Draws centered menu, pause, win, and game over messages.
void Game::drawCenteredMessage(const sf::String& message, float y)
{
    if (!fontLoaded)
        return;

    sf::Text shadow(font, message, 46);
    shadow.setFillColor(sf::Color(0, 0, 0, 160));
    shadow.setStyle(sf::Text::Bold);
    

    const sf::FloatRect shadowBounds = shadow.getLocalBounds();

    shadow.setOrigin({
        shadowBounds.position.x + shadowBounds.size.x / 2.f,
        shadowBounds.position.y + shadowBounds.size.y / 2.f
     });

    shadow.setPosition({
        Constants::WindowWidth / 2.f + 3.f,
        y + 3.f
    });

    sf::Text text(font, message, 46);
    text.setFillColor(sf::Color(210, 235, 255));
    text.setStyle(sf::Text::Bold);

    const sf::FloatRect textBounds = text.getLocalBounds();

    text.setOrigin({
        textBounds.position.x + textBounds.size.x / 2.f,
        textBounds.position.y + textBounds.size.y / 2.f
    });

    text.setPosition({
        Constants::WindowWidth / 2.f,
        y
    });

    window.draw(shadow);
    window.draw(text);
}

// Creates a falling bonus at the destroyed bonus block position.
void Game::spawnBonusFromBrick(const Brick& brick)
{
    const sf::FloatRect bounds = brick.getBounds();

    const sf::Vector2f bonusPosition{
        bounds.position.x + bounds.size.x / 2.f,
        bounds.position.y + bounds.size.y / 2.f
    };

    BonusType type;

    if (bonusSpawnCounter % 2 == 0)
        type = BonusType::ExpandPaddle;
    else
        type = BonusType::ExtraBall;

    ++bonusSpawnCounter;

    bonuses.emplace_back(bonusPosition, type);
}

// Updates falling bonuses and applies them when collected by the paddle.
void Game::updateBonuses(float deltaTime)
{
    for (Bonus& bonus : bonuses)
    {
        bonus.update(deltaTime);

        if (bonus.getBounds().findIntersection(paddle.getBounds()).has_value())
        {
            applyBonus(bonus.getType());

            bonus = Bonus({ -100.f, Constants::WindowHeight + 100.f }, bonus.getType());
        }
    }

    bonuses.erase(
        std::remove_if(
            bonuses.begin(),
            bonuses.end(),
            [](const Bonus& bonus)
            {
                return bonus.isOutOfScreen();
            }
        ),
        bonuses.end()
    );
}

// Applies the collected bonus effect.
void Game::applyBonus(BonusType type)
{
    switch (type)
    {
    case BonusType::ExpandPaddle:
    {
        if (paddleBonusLevel < Constants::MaxPaddleBonusLevel)
            ++paddleBonusLevel;

        paddleBonusTimer = Constants::PaddleBonusDuration;

        const float widthMultiplier = 1.f + paddleBonusLevel * 0.35f;
        paddle.setWidth(Constants::PaddleWidth * widthMultiplier);

        break;
    }

    // Spawn temporary extra balls with slightly different trajectories.
    case BonusType::ExtraBall:
    {
        for (int i = 0; i < Constants::ExtraBallsPerBonus; ++i)
        {
            if (static_cast<int>(extraBalls.size()) >= Constants::MaxExtraBalls)
                break;

                Ball newBall;

                newBall.setVisualColors(
                    sf::Color(40, 210, 255),
                    sf::Color(120, 230, 255, 95),
                    sf::Color(80, 210, 255, 45)
                );

                const std::size_t ballIndex = extraBalls.size();

                const float direction = ballIndex % 2 == 0 ? 1.f : -1.f;
                const float horizontalSpeed = 150.f + 25.f * static_cast<float>(ballIndex);

                newBall.setPosition(
                    paddle.getCenterX(),
                    Constants::PaddleY - 35.f
                );

                newBall.setVelocity(
                    direction * horizontalSpeed,
                    Constants::BallStartSpeedY
                );

                extraBalls.push_back(newBall);
        }
            extraBallTimer = Constants::ExtraBallDuration;
    }
        break;
    }
}

void Game::updatePaddleBonusTimer(float deltaTime)
{
    if (paddleBonusLevel <= 0)
        return;

    paddleBonusTimer -= deltaTime;

    if (paddleBonusTimer <= 0.f)
    {
        paddleBonusLevel = 0;
        paddleBonusTimer = 0.f;
        paddle.setWidth(Constants::PaddleWidth);
    }
}

void Game::updateExtraBallTimer(float deltaTime)
{
    if (extraBalls.empty())
        return;

    extraBallTimer -= deltaTime;

    if (extraBallTimer <= 0.f)
    {
        extraBalls.clear();
        extraBallTimer = 0.f;
    }
}

// Draws the static background and playfield frame.
void Game::drawBackground()
{
    // Main vertical gradient
    sf::VertexArray gradient;
    gradient.setPrimitiveType(sf::PrimitiveType::TriangleStrip);
    gradient.resize(4);

    gradient[0].position = { 0.f, 0.f };
    gradient[1].position = { static_cast<float>(Constants::WindowWidth), 0.f };
    gradient[2].position = { 0.f, static_cast<float>(Constants::WindowHeight) };
    gradient[3].position = { static_cast<float>(Constants::WindowWidth), static_cast<float>(Constants::WindowHeight) };

    gradient[0].color = sf::Color(12, 16, 35);
    gradient[1].color = sf::Color(12, 16, 35);
    gradient[2].color = sf::Color(4, 6, 14);
    gradient[3].color = sf::Color(4, 6, 14);

    window.draw(gradient);

    // Soft playyfield panel
    sf::RectangleShape playfield;
    playfield.setSize({
        static_cast<float>(Constants::WindowWidth) - 160.f,
        static_cast<float>(Constants::WindowHeight) - 190.f
    });
    playfield.setPosition({ 80.f, 90.f });
    playfield.setFillColor(sf::Color(255, 255, 255, 8));
    playfield.setOutlineThickness(1.f);
    playfield.setOutlineColor(sf::Color(120, 170, 255, 28));

    window.draw(playfield);

    // Top glow
    sf::CircleShape topGlow(240.f);
    topGlow.setOrigin({ 240.f, 240.f });
    topGlow.setPosition({
        Constants::WindowWidth / 2.f,
        20.f
    });
    topGlow.setScale({ 2.0f, 0.35f });
    topGlow.setFillColor(sf::Color(80, 140, 255, 18));

    window.draw(topGlow);

    // Decorative background lines
    for (int i = 0; i < 8; ++i)
    {
        sf::RectangleShape line;
        line.setSize({ static_cast<float>(Constants::WindowWidth) - 160.f, 1.f });
        line.setPosition({ 80.f, 110.f * i * 55.f });
        line.setFillColor(sf::Color(255, 255, 255, 6));

        window.draw(line);
    }

    // Small glowing doots
    for (int i = 0; i < 16; ++i)
    {
        const float x = 70.f + static_cast<float>((i * 113) % (Constants::WindowWidth - 140));
        const float y = 95.f + static_cast<float>((i * 71) % (Constants::WindowHeight - 200));

        sf::CircleShape dot(2.f);
        dot.setOrigin({ 2.f, 2.f });
        dot.setPosition({ x, y });
        dot.setFillColor(sf::Color(140, 190, 255, 45));

        window.draw(dot);
    }
}

// Checks if a moving brick would collide with another block or the screen edge.
bool Game::isBrickMovementBlocked(
    std::size_t movingBrickIndex,
    float offsetX,
    std::size_t& blockingBrickIndex
) const
{
    const Brick& movingBrick = bricks[movingBrickIndex];

    sf::FloatRect futureBounds = movingBrick.getBounds();
    futureBounds.position.x += offsetX;

    const float levelWidth =
        static_cast<float>(Constants::LevelColumns) * Constants::BrickWidth;

    const float levelLeft =
        (static_cast<float>(Constants::WindowWidth) - levelWidth) / 2.f;

    const float levelRight =
        levelLeft + levelWidth;

    if (futureBounds.position.x < levelLeft)
    {
        blockingBrickIndex = bricks.size();
        return true;
    }

    if (futureBounds.position.x + futureBounds.size.x > levelRight)
    {
        blockingBrickIndex = bricks.size();
        return true;
    }

    for (std::size_t i = 0; i < bricks.size(); ++i)
    {
        if (i == movingBrickIndex)
            continue;

        const Brick& otherBrick = bricks[i];

        if (otherBrick.isDestroyed())
            continue;

        if (futureBounds.findIntersection(otherBrick.getBounds()).has_value())
        {
            blockingBrickIndex = i;
            return true;
        }
    }

    return false;
}

// Moves metal blocks and reverses them when blocked.
void Game::updateMovingBricks(float deltaTime)
{
    for (std::size_t i = 0; i < bricks.size(); ++i)
    {
        Brick& brick = bricks[i];

        if (brick.isDestroyed())
            continue;

        if (!brick.isMetal() || !brick.isMoving())
            continue;

        const float offsetX = 
            brick.getMoveDirection() * brick.getMoveSpeed() * deltaTime;

        std::size_t blockingBrickIndex = bricks.size();

        const bool blockedForward =
            isBrickMovementBlocked(i, offsetX, blockingBrickIndex);

        if (!blockedForward)
        {
            brick.move({ offsetX, 0.f });
            continue;
        }

        if (blockingBrickIndex < bricks.size())
        {
            Brick& blockingBrick = bricks[blockingBrickIndex];

            if (blockingBrick.isMetal() && blockingBrick.isMoving())
            {
                const sf::FloatRect brickBounds = brick.getBounds();
                const sf::FloatRect blockingBounds = blockingBrick.getBounds();

                const float brickCenter =
                    brickBounds.position.x + brickBounds.size.x / 2.f;

                const float blockingCenter =
                    blockingBounds.position.x + blockingBounds.size.x / 2.f;

                if (brickCenter < blockingCenter)
                {
                    brick.setMoveDirection(-1.f);
                    blockingBrick.setMoveDirection(1.f);
                }
                else
                {
                    brick.setMoveDirection(1.f);
                    blockingBrick.setMoveDirection(-1.f);
                }

                const float newOffsetX =
                    brick.getMoveDirection() * brick.getMoveSpeed() * deltaTime;

                std::size_t newBlockingIndex = bricks.size();

                if (!isBrickMovementBlocked(i, newOffsetX, newBlockingIndex))
                    brick.move({ newOffsetX, 0.f });

                continue;
            }
        }

        const float oppositeOffsetX = -offsetX;

        std::size_t oppositeBlockingBrickIndex = bricks.size();

        const bool blockedOpposite =
            isBrickMovementBlocked(i, oppositeOffsetX, oppositeBlockingBrickIndex);

        if (!blockedOpposite)
        {
            brick.setMoveDirection(-brick.getMoveDirection());
            brick.move({ oppositeOffsetX, 0.f });
            continue;
        }
    }
}

// Spawns small particles for regular impacts.
void Game::spawnImpactSparks(sf::Vector2f position)
{
    const sf::Vector2f velocities[] =
    {
        {-120.f, -80.f},
        {-80.f, -130.f},
        {-30.f, -100.f},
        {40.f, -120.f},
        {100.f, -90.f},
        {130.f, -30.f},
        {-130.f, 20.f},
        {90.f, 45.f}
    };

    for (const sf::Vector2f& velocity : velocities)
    {
        Spark spark;
        spark.position = position;
        spark.velocity = velocity;
        spark.color = sf::Color(220, 235, 255, 180);
        spark.lifetime = 0.28f;
        spark.maxLifetime = 0.28f;
        spark.radius = 2.f;

        sparks.push_back(spark);
    }
}

// Spawns stronger particles for fireball impacts.
void Game::spawnFireImpactSparks(sf::Vector2f position)
{
    const sf::Vector2f velocities[] =
    {
        {-210.f, -120.f},
        {-170.f, -190.f},
        {-100.f, -240.f},
        {-35.f, -180.f},
        {45.f, -230.f},
        {115.f, -170.f},
        {190.f, -130.f},
        {230.f, -40.f},
        {-230.f, 20.f},
        {-150.f, 80.f},
        {-60.f, 120.f},
        {70.f, 115.f},
        {160.f, 65.f},
        {220.f, 20.f}
    };

    for (int i = 0; i < 14; ++i)
    {
        Spark spark;
        spark.position = position;
        spark.velocity = velocities[i];

        if (i % 3 == 0)
            spark.color = sf::Color(255, 230, 120, 220);
        else if (i % 3 == 1)
            spark.color = sf::Color(255, 130, 45, 210);
        else
            spark.color = sf::Color(255, 70, 35, 190);

        spark.lifetime = 0.42f;
        spark.maxLifetime = 0.42f;
        spark.radius = 3.f;

        sparks.push_back(spark);
    }
}

// Updates particle positions and removes expired particles.
void Game::updateSparks(float deltaTime)
{
    for (Spark& spark : sparks)
    {
        spark.lifetime -= deltaTime;

        spark.position += spark.velocity * deltaTime;
        spark.velocity.y += 260.f * deltaTime;
    }

    sparks.erase(
        std::remove_if(
            sparks.begin(),
            sparks.end(),
            [](const Spark& spark)
            {
                return spark.lifetime <= 0.f;
            }
        ),
        sparks.end()
    );
}

// Draws active particles with fading alpha.
void Game::drawSparks()
{
    for (const Spark& spark : sparks)
    {
        const float alphaRatio = spark.lifetime / spark.maxLifetime;

        const std::uint8_t alpha = 
            static_cast<std::uint8_t>(static_cast<float>(spark.color.a) * alphaRatio);

        sf::CircleShape particle(spark.radius);
        particle.setOrigin({ spark.radius, spark.radius });
        particle.setPosition(spark.position);

        particle.setFillColor(sf::Color(
            spark.color.r,
            spark.color.g,
            spark.color.b,
            alpha
        ));

        window.draw(particle);
    }
}

// Updates fire cloud animation and periodic spawning.
void Game::updateFireClouds(float deltaTime)
{
    fireCloudVisualTimer += deltaTime;
    fireCloudSpawnTimer += deltaTime;

    if (fireCloudSpawnTimer >= Constants::FireCloudSpawnInterval)
    {
        fireCloudSpawnTimer = 0.f;

        if (static_cast<int>(fireClouds.size()) < Constants::MaxFireClouds)
        {
            trySpawnFireCloud();
        }
    }
}

void Game::drawFireClouds()
{
    constexpr float Pi = 3.14159265f;
    
    for (const FireCloud& cloud : fireClouds)
    {
        const float pulse =
            1.f + 0.08f * std::sin(fireCloudVisualTimer * 2.2f + cloud.pulsePhase);

        const float flicker =
            0.85f + 0.15f * std::sin(fireCloudVisualTimer * 5.5f + cloud.flickerPhase);

        const float outerRadius = cloud.radius * pulse;
        const float middleRadius = cloud.radius * 0.68f * pulse;
        const float coreRadius = cloud.radius * 0.34f * pulse;

        const std::uint8_t outerAlpha =
            static_cast<std::uint8_t>(35.f * flicker);

        const std::uint8_t middleAlpha =
            static_cast<std::uint8_t>(55.f * flicker);

        const std::uint8_t coreAlpha =
            static_cast<std::uint8_t>(78.f * flicker);

        sf::CircleShape halo(outerRadius * 1.18f);
        halo.setOrigin({ outerRadius * 1.18f, outerRadius * 1.18f });
        halo.setPosition(cloud.position);
        halo.setFillColor(sf::Color(255, 90, 40, static_cast<std::uint8_t>(18.f * flicker)));
        
        sf::CircleShape outer(outerRadius);
        outer.setOrigin({ outerRadius, outerRadius });
        outer.setPosition(cloud.position);
        outer.setFillColor(sf::Color(255, 120, 60, outerAlpha));

        sf::CircleShape middle(middleRadius);
        middle.setOrigin({ middleRadius, middleRadius });
        middle.setPosition(cloud.position);
        middle.setFillColor(sf::Color(255, 190, 90, middleAlpha));

        sf::CircleShape core(coreRadius);
        core.setOrigin({ coreRadius, coreRadius });
        core.setPosition(cloud.position);
        core.setFillColor(sf::Color(255, 235, 170, coreAlpha));

        window.draw(halo);
        window.draw(outer);
        window.draw(middle);
        window.draw(core);

        for (int i = 0; i < Constants::FireCloudSwirlParticleCount; ++i)
        {
            const float normalizedIndex =
                static_cast<float>(i) / static_cast<float>(Constants::FireCloudSwirlParticleCount);

            const float angle =
                normalizedIndex * Pi * 2.f +
                fireCloudVisualTimer * Constants::FireCloudSwirlSpeed +
                cloud.pulsePhase;

            const float orbitPulse =
                1.f + 0.12f * std::sin(
                    fireCloudVisualTimer * 3.f +
                    static_cast<float>(i) * 0.7f +
                    cloud.flickerPhase
                );

            const float orbitRadius =
                Constants::FireCloudSwirlRadius * orbitPulse;

            const sf::Vector2f particlePosition{
                cloud.position.x + std::cos(angle) * orbitRadius,
                cloud.position.y + std::sin(angle) * orbitRadius * 0.72f
            };

            const float particleRadius =
                3.f + 1.5f * std::sin(
                    fireCloudVisualTimer * 4.f +
                    static_cast<float>(i)
                );

            const std::uint8_t particleAlpha =
                static_cast<std::uint8_t>(90.f * flicker);

            sf::CircleShape particle(particleRadius);
            particle.setOrigin({ particleRadius, particleRadius });
            particle.setPosition(particlePosition);

            if (i % 2 == 0)
            {
                particle.setFillColor(sf::Color(
                    255,
                    190,
                    80,
                    particleAlpha
                ));
            }
            else
            {
                particle.setFillColor(sf::Color(
                    255,
                    90,
                    35,
                    particleAlpha
                ));
            }

            window.draw(particle);
        }
    }
}

// Validates that a fire cloud is reachable and not too close to blocks or other clouds.
bool Game::isValidFireCloudPosition(sf::Vector2f position) const
{
    const float safeDistanceFromBlocks = Constants::BrickWidth * 2.2f;
    const float usefulDistanceFromBlocks = Constants::BrickWidth * 8.f;

    bool hasUsefulBlockNearby = false;

    for (const FireCloud& cloud : fireClouds)
    {
        const sf::Vector2f delta{
            position.x - cloud.position.x,
            position.y - cloud.position.y
        };

        const float distanceSquared =
            delta.x * delta.x + delta.y * delta.y;

        const float minDistance =
            Constants::FireCloudMinDistanceFromOtherClouds;

        if (distanceSquared < minDistance * minDistance)
            return false;
    }

    for (const Brick& brick : bricks)
    {
        if (brick.isDestroyed())
            continue;

        const sf::FloatRect bounds = brick.getBounds();

        const sf::Vector2f brickCenter{
            bounds.position.x + bounds.size.x / 2.f,
            bounds.position.y + bounds.size.y / 2.f
        };

        const sf::Vector2f delta{
            position.x - brickCenter.x,
            position.y - brickCenter.y
        };

        const float distanceSquared =
            delta.x * delta.x + delta.y * delta.y;

        if (distanceSquared < safeDistanceFromBlocks * safeDistanceFromBlocks)
            return false;

        if (distanceSquared < usefulDistanceFromBlocks * usefulDistanceFromBlocks)
            hasUsefulBlockNearby = true;
    }

    return hasUsefulBlockNearby;
}

// Attempts to place a new fire cloud in a useful and safe position.
void Game::trySpawnFireCloud()
{
    static std::mt19937 generator{ std::random_device{}() };

    const float levelWidth =
        static_cast<float>(Constants::LevelColumns) * Constants::BrickWidth;

    const float levelLeft =
        (static_cast<float>(Constants::WindowWidth) - levelWidth) / 2.f;

    const float levelRight =
        levelLeft + levelWidth;

    const float minX = levelLeft + Constants::FireCloudRadius;
    const float maxX = levelRight - Constants::FireCloudRadius;

    const float minY = 160.f;
    const float maxY = Constants::PaddleY - 180.f;

    std::uniform_real_distribution<float> xDistribution(minX, maxX);
    std::uniform_real_distribution<float> yDistribution(minY, maxY);
    std::uniform_real_distribution<float> phaseDistribution(0.f, 6.2831853f);

    constexpr int MaxAttempts = 60;

    for (int attempt = 0; attempt < MaxAttempts; ++attempt)
    {
        const sf::Vector2f position{
            xDistribution(generator),
            yDistribution(generator)
        };

        if (!isValidFireCloudPosition(position))
            continue;

        FireCloud cloud;
        cloud.position = position;
        cloud.radius = Constants::FireCloudRadius;
        cloud.pulsePhase = phaseDistribution(generator);
        cloud.flickerPhase = phaseDistribution(generator);

        fireClouds.push_back(cloud);
        return;
    }
}

// Grants fireball status when a ball touches a fire cloud.
void Game::checkFireCloudCollision(Ball& targetBall)
{
    const sf::FloatRect ballBounds = targetBall.getBounds();

    const sf::Vector2f ballCenter{
        ballBounds.position.x + ballBounds.size.x / 2.f,
        ballBounds.position.y + ballBounds.size.y / 2.f
    };

    for (std::size_t i = 0; i < fireClouds.size(); ++i)
    {
        const FireCloud& cloud = fireClouds[i];

        const sf::Vector2f delta{
            ballCenter.x - cloud.position.x,
            ballCenter.y - cloud.position.y
        };

        const float distanceSquared =
            delta.x * delta.x + delta.y * delta.y;

        const float activationRadius =
            cloud.radius + Constants::BallRadius;

        if (distanceSquared <= activationRadius * activationRadius)
        {
            // Fire clouds cleanse slow before applying fireball status.
            if (targetBall.isSlowed())
                targetBall.consumeSlow();
            
            targetBall.activateFireball(Constants::FireballDuration);

            spawnImpactSparks(cloud.position);

            fireClouds.erase(fireClouds.begin() + static_cast<std::ptrdiff_t>(i));
            return;
        }
    }
}

// Teleports a ball to another portal of the same group.
void Game::teleportBallThroughPortal(Ball& targetBall, Brick& entryPortal)
{
    // Avoid repeated teleporting while the ball is still near a portal.
    if (!targetBall.canTeleport())
        return;

    // Find another portal with the same group.
    Brick* exitPortal = nullptr;
    const int portalGroup = entryPortal.getPortalGroup();

    for (Brick& brick : bricks)
    {
        if (brick.isDestroyed())
            continue;

        if (!brick.isPortal())
            continue;

        if (&brick == &entryPortal)
            continue;

        if (brick.getPortalGroup() != portalGroup)
            continue;

        exitPortal = &brick;
        break;
    }

    if (exitPortal == nullptr)
        return;

    entryPortal.triggerPortalFlash();
    exitPortal->triggerPortalFlash();

    const sf::FloatRect exitBounds = exitPortal->getBounds();

    const sf::Vector2f exitCenter{
        exitBounds.position.x + exitBounds.size.x / 2.f,
        exitBounds.position.y + exitBounds.size.y / 2.f
    };

    sf::Vector2f direction = targetBall.getVelocity();

    const float length =
        std::sqrt(direction.x * direction.x + direction.y * direction.y);

    if (length > 0.f)
    {
        direction.x /= length;
        direction.y /= length;
    }
    else
    {
        direction = { 0.f, 1.f };
    }

    // Place the ball slightly outside the exit portal to avoid instant re-entry.
    const float exitOffset =
        std::max(exitBounds.size.x, exitBounds.size.y) / 2.f +
        Constants::BallRadius +
        12.f;

    const sf::Vector2f exitPosition{
        exitCenter.x + direction.x * exitOffset,
        exitCenter.y + direction.y * exitOffset
    };

    targetBall.setPosition(exitPosition.x, exitPosition.y);
    targetBall.triggerTeleportCooldown(Constants::PortalTeleportCooldown);

    const sf::FloatRect entryBounds = entryPortal.getBounds();

    const sf::Vector2f entryCenter{
        entryBounds.position.x + entryBounds.size.x / 2.f,
        entryBounds.position.y + entryBounds.size.y / 2.f
    };

    spawnPortalSparks(
        entryCenter,
        entryPortal.getPortalPrimaryColor(),
        entryPortal.getPortalSecondaryColor()
    );

    spawnPortalSparks(
        exitCenter,
        exitPortal->getPortalPrimaryColor(),
        exitPortal->getPortalSecondaryColor()
    );
}

// Spawns colored particles at portal entry and exit points.
void Game::spawnPortalSparks(
    sf::Vector2f position,
    sf::Color primaryColor,
    sf::Color secondaryColor
)
{
    const sf::Vector2f velocities[] =
    {
        {-170.f, -120.f},
        {-120.f, -190.f},
        {-60.f, -145.f},
        {50.f, -170.f},
        {130.f, -120.f},
        {180.f, -35.f},
        {-180.f, 20.f},
        {-110.f, 95.f},
        {85.f, 110.f},
        {170.f, 35.f}
    };

    for (int i = 0; i < 10; ++i)
    {
        Spark spark;
        spark.position = position;
        spark.velocity = velocities[i];
        spark.lifetime = 0.35f;
        spark.maxLifetime = 0.35f;
        spark.radius = 2.5f;

        if (i % 2 == 0)
            spark.color = sf::Color(primaryColor.r, primaryColor.g, primaryColor.b, 210);
        else
            spark.color = sf::Color(secondaryColor.r, secondaryColor.g, secondaryColor.b, 190);

        sparks.push_back(spark);
    }
}

// Triggers ghost block feedback and schedules delayed ball trajectory change.
void Game::handleGhostBlockCollision(Ball& targetBall, Brick& ghostBlock)
{
    ghostBlock.triggerGhostFlash();

    if (!targetBall.canTriggerGhostBlock())
        return;

    targetBall.triggerDelayedTrajectoryShift(
        Constants::GhostBlockTrajectoryDelay
    );
}

// Destroys the generator, removes linked ghost blocks, and damages nearby blocks.
void Game::explodeGenerator(Brick& generatorBrick)
{
    const int group = generatorBrick.getLinkGroup();
    const sf::FloatRect generatorBounds = generatorBrick.getBounds();

    const sf::Vector2f generatorCenter{
        generatorBounds.position.x + generatorBounds.size.x / 2.f,
        generatorBounds.position.y + generatorBounds.size.y / 2.f
    };

    generatorBrick.destroy();
    spawnFireImpactSparks(generatorCenter);

    for (Brick& brick : bricks)
    {
        if (brick.isDestroyed())
            continue;

        if (&brick == &generatorBrick)
            continue;

        // Remove only ghost blocks linked to this generator group.
        if (brick.isGhostBlock() && brick.getLinkGroup() == group)
        {
            brick.destroy();
            continue;
        }

        // Trigger chain reactions when another generator is caught in the explosion.
        if (brick.isGenerator())
        {
            explodeGenerator(brick);
            continue;
        }

        // Explosion does not affect permanent interactive blocks.
        if (brick.isMetal() || brick.isBumper() || brick.isPortal() || brick.isGhostBlock())
            continue;

        const sf::FloatRect bounds = brick.getBounds();

        const sf::Vector2f brickCenter{
            bounds.position.x + bounds.size.x / 2.f,
            bounds.position.y + bounds.size.y / 2.f
        };

        const float dx = std::abs(brickCenter.x - generatorCenter.x);
        const float dy = std::abs(brickCenter.y - generatorCenter.y);

        if (dx <= Constants::GeneratorExplosionRadiusX &&
            dy <= Constants::GeneratorExplosionRadiusY)
        {
            const int brickTypeBeforeExplosion = brick.getType();

            brick.destroy();

            score += 80;

            if (brickTypeBeforeExplosion == 2)
                spawnBonusFromBrick(brick);

            spawnImpactSparks(brickCenter);
        }
    }

    score += 200;
}