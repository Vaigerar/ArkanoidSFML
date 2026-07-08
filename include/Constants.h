#pragma once

namespace Constants
{
    // Window and rendering resolution.
    constexpr unsigned WindowWidth = 1920;
    constexpr unsigned WindowHeight = 1080;

    // Main ball settings.
    constexpr float BallRadius = 8.f;
    constexpr float BallStartX = WindowWidth / 2.f;
    constexpr float BallStartY = WindowHeight - 85.f;
    constexpr float BallStartSpeedY = -360.f;

    // Player paddle settings.
    constexpr float PaddleWidth = 180.f;
    constexpr float PaddleHeight = 18.f;
    constexpr float PaddleY = WindowHeight - 55.f;
    constexpr float PaddleSpeed = 880.f;

    // Level grid layout.
    constexpr int LevelColumns = 32;
    constexpr int LevelRows = 18;

    constexpr float BrickWidth = 48.f;
    constexpr float BrickHeight = 24.f;

    constexpr float ShadowOffsetX = 4.f;
    constexpr float ShadowOffsetY = 4.f;
    constexpr float GlowSizeSmall = 4.f;
    constexpr float GlowSizeMedium = 8.f;

    // Bonus and temporary ball effect settings.
    constexpr int MaxExtraBalls = 8;
    constexpr int ExtraBallsPerBonus = 2;

    constexpr int MaxPaddleBonusLevel = 3;
    constexpr float PaddleBonusDuration = 10.f;
    constexpr float ExtraBallDuration = 12.f;

    // Special block movement and size settings.
    constexpr float BumperWidth = 22.f;
    constexpr float BumperHeight = BrickHeight * 3.f;

    constexpr float MetalBlockMoveSpeed = 230.f;

    constexpr float BumperMoveSpeed = 115.f;
    constexpr float BumperMoveRange = BrickHeight * 3.f;

    constexpr float BallSpeedBoostMultiplier = 1.45f;
    constexpr float BallSpeedBoostDuration = 4.f;

    // Fire cloud and fireball settings.
    constexpr float FireCloudSpawnInterval = 30.f;
    constexpr int MaxFireClouds = 2;
    constexpr float FireCloudRadius = 42.f;
    constexpr float FireCloudMinDistanceFromOtherClouds = 260.f;

    constexpr int FireCloudSwirlParticleCount = 8;
    constexpr float FireCloudSwirlRadius = 54.f;
    constexpr float FireCloudSwirlSpeed = 1.8f;

    constexpr float FireballDuration = 10.f;

    // Portal teleportation settings.
    constexpr float PortalTeleportCooldown = 0.7f;

    // Ice block slow effect.
    constexpr float IceBlockSlowMultiplier = 0.55f;
    constexpr float IceBlockSlowDuration = 4.5f;

    // Ghost block delayed trajectory effect.
    constexpr float GhostBlockTrajectoryDelay = 1.f;
    constexpr float GhostBlockTriggerCooldown = 1.2f;
   
    // Generator explosion settings.
    constexpr float GeneratorExplosionRadiusX = BrickWidth * 1.35f;
    constexpr float GeneratorExplosionRadiusY = BrickHeight * 1.8f;

    constexpr int StartLives = 3;
}
