# Arkanoid SFML

A modern Arkanoid-style game written in **C++17** with **SFML 3**.

This project is a clean Visual Studio / SFML rewrite of an older C++Builder/VCL Arkanoid project. The goal is to keep the classic Arkanoid base, but expand it with modern effects, special block mechanics, bonuses, portals, generators, and temporary ball statuses.

## Features

- Fullscreen 1920x1080 gameplay
- Paddle movement with `A / D` or `Left / Right`
- Angle-based paddle bounce
- Main ball and temporary extra balls
- Lives and score system
- Pause, restart, win, and game over states
- Bonus blocks with falling bonuses
- Expand paddle bonus with stacking levels
- Extra ball bonus with limited lifetime
- Strong blocks with damaged visual state
- Moving metal blocks
- Bumper blocks that boost the ball
- Fire clouds that grant fireball status
- Fireball interactions with metal and destructible blocks
- Ice blocks that slow the ball
- Portal groups with teleport cooldown
- Generator blocks with explosion and chain reactions
- Ghost blocks linked to generators
- Particle effects for impacts, fire, portals, and special interactions
- Class-based architecture: `Game`, `Ball`, `Paddle`, `Brick`, `Bonus`

## Controls

| Key | Action |
| --- | --- |
| `Enter` | Start the game from the main menu / restart after win or game over |
| `A` or `Left Arrow` | Move paddle left |
| `D` or `Right Arrow` | Move paddle right |
| `Space` | Pause / resume |
| `Escape` | Close the game |

## Block Legend

The level is stored as a `32 x 18` character map in `src/Game.cpp`.

| Symbol | Block Type | Description |
| --- | --- | --- |
| `0` | Empty cell | No block is created. |
| `1` | Normal block | Destroyed by one normal hit. |
| `2` | Bonus block | Drops a random bonus when destroyed. |
| `3` | Strong block | Requires two normal hits. Changes appearance after the first hit. |
| `4` | Damaged strong block | Internal damaged state. Usually not placed manually in the level map. |
| `5` | Moving metal block | Moves horizontally. Normal balls cannot destroy it. Fireball damages it first, then destroys it on the next fire hit. |
| `6` | Bumper block | Non-destructible. Cleanses slow, boosts the ball, and consumes fireball status. |
| `7` | Portal group 1 | Teleports the ball to another `7` portal. |
| `8` | Portal group 2 | Teleports the ball to another `8` portal. |
| `9` | Portal group 3 | Teleports the ball to another `9` portal. |
| `A` | Ice block | Applies slow when destroyed by a non-fireball hit. Fireball destroys it without slow. |
| `B` | Generator group B | Explodes, destroys nearby destructible blocks, and removes linked `b` ghost blocks. |
| `b` | Ghost block group B | Pass-through trigger. Schedules a delayed trajectory change for the ball. Removed by `B`. |
| `C` | Generator group C | Same as `B`, but linked to `c` ghost blocks. |
| `c` | Ghost block group C | Same as `b`, but linked to `C`. |
| `D` | Generator group D | Same as `B`, but linked to `d` ghost blocks. |
| `d` | Ghost block group D | Same as `b`, but linked to `D`. |

## Main Mechanics

### Fireball

Fire clouds periodically appear on the level. When any ball touches a fire cloud, it receives fireball status for a limited time.

Fireball can:

- destroy normal, bonus, strong, and ice blocks without applying ice slow;
- damage metal blocks;
- destroy already damaged metal blocks;
- get consumed by bumpers and valid special interactions.

### Ice Slow

Ice blocks slow the ball when destroyed by a non-fireball hit.

Slow can be removed by:

- waiting until the timer expires;
- hitting a bumper block;
- touching a fire cloud.

### Portals

Portals are grouped by symbol:

- `7` teleports to another `7`
- `8` teleports to another `8`
- `9` teleports to another `9`

After teleportation, the ball receives a short cooldown to prevent instant repeated teleporting.

### Generators and Ghost Blocks

Generator blocks and ghost blocks are connected through link groups:

- `B` controls `b`
- `C` controls `c`
- `D` controls `d`

Ghost blocks are not solid. Balls pass through them, but the ghost block schedules a delayed horizontal direction change. When the matching generator is destroyed, all linked ghost blocks disappear.

Generator explosions can also trigger nearby generators, creating chain reactions.

## Project Structure

```text
ArkanoidSFML/
├── CMakeLists.txt
├── README.md
├── include/
│   ├── Ball.h
│   ├── Bonus.h
│   ├── Brick.h
│   ├── Constants.h
│   ├── Game.h
│   └── Paddle.h
└── src/
    ├── Ball.cpp
    ├── Bonus.cpp
    ├── Brick.cpp
    ├── Game.cpp
    ├── main.cpp
    └── Paddle.cpp
```

## Requirements

- Visual Studio 2022 with **Desktop development with C++**
- CMake 3.20 or newer
- vcpkg
- SFML 3
- C++17 compiler

## Build with Visual Studio + vcpkg

1. Install Visual Studio 2022 with **Desktop development with C++**.
2. Install and bootstrap vcpkg.
3. Install SFML:

```powershell
vcpkg install sfml:x64-windows
```

4. Open this project folder in Visual Studio:

```text
File -> Open -> Folder...
```

5. Configure CMake. If needed, pass the vcpkg toolchain manually:

```powershell
cmake -B build -S . -DCMAKE_TOOLCHAIN_FILE=C:/vcpkg/scripts/buildsystems/vcpkg.cmake
```

6. Build the project:

```powershell
cmake --build build --config Release
```

7. Run `ArkanoidSFML`.

## Font

The game currently tries to load Arial from:

```text
C:/Windows/Fonts/arial.ttf
```

If the font is not available, the game should still run, but text UI may be hidden.

## Notes for Level Editing

- Each level row must contain exactly **32 characters**.
- The level must contain exactly **18 rows**.
- Keep `4` out of manual level layouts unless you specifically want to spawn a damaged strong block state.
- Ghost blocks are linked by their lowercase symbol: `b`, `c`, `d`.
- Generator blocks are linked by their uppercase symbol: `B`, `C`, `D`.
- If you want the player to discover generator links, keep all ghost block visuals identical.

## Current Status

This is a playable prototype focused on mechanics and visual feedback. The code is intentionally kept simple and class-based so new blocks, bonuses, and visual effects can be added step by step.
