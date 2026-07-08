cmake_minimum_required(VERSION 3.20)
project(ArkanoidSFML LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(SFML 3 REQUIRED COMPONENTS Graphics Window System)

add_executable(ArkanoidSFML
    src/main.cpp
    src/Game.cpp
    src/Ball.cpp
    src/Paddle.cpp
    src/Brick.cpp
    src/Bonus.cpp
 )

target_include_directories(ArkanoidSFML PRIVATE include)
target_link_libraries(ArkanoidSFML PRIVATE SFML::Graphics SFML::Window SFML::System)

# For Visual Studio: set ArkanoidSFML as startup project when opening CMake folder.
set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT ArkanoidSFML)
