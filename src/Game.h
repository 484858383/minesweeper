#pragma once

#include<iostream>
#include<vector>
#include<cstdint>

#include<SFML/Graphics.hpp>

enum class Cell : int8_t
{
	empty = 0,
	one, two, three, four,
	five, six, seven, eight,
	blank = 9,
	flag = 10,
	mine = 11,
};

class Game
{
public:
	Game();

	void run();
private:
	void handleEvents();
	void update();

	void generate();
	void changeCell(int x, int y);


	int index(int x, int y);
private:
	sf::Vector2i m_size;
	sf::VertexArray m_cellsVAO;
	sf::RenderWindow m_window;
	sf::Texture m_gameTexture;
	sf::Font m_font;

	std::vector<Cell> m_cells;
	std::vector<Cell> m_playerCells;
};