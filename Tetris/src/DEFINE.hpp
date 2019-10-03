#pragma once

#define APP_NAME "Tetris"

#define FONT_PATH "res/fonts/FreePixel.ttf"

#define APP_HEIGHT 900
#define APP_WIDTH 800

#define BLOCK_SIZE 30

#define GRID_START_POS_X 250
#define GRID_START_POS_Y 100
#define GRID_END_POS_Y 700
#define GRID_END_POS_X 550

#define EMPTY_GRID_PLACE std::pair<bool, sf::RectangleShape>(false, sf::RectangleShape())
#define FILLED_GRID_PLACE std::pair<bool, sf::RectangleShape>(true, sf::RectangleShape(sf::Vector2f(BLOCK_SIZE, BLOCK_SIZE)))

#define to_uns(x) static_cast<unsigned>(x)