#include <SFML/Graphics.hpp>

#include "Game.hpp"

#include <iostream>

/*
TODO :
    * arme spéciale
*/

int main(int argc, const char *argv[])
{
    Game g("config.txt");
    g.run();

    return 0;
}