#include "Game.h"
#include <iostream>

int main()
{
    try
    {
        // Create the game instance and start the main loop.
        Game game;
        game.run();
    }
    catch (const std::exception& exception)
    {
        std::cerr << "Error: " << exception.what() << '\n';
        return 1;
    }

    return 0;
}
