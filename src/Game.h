#pragma once

#include<iostream>
#include<vector>
#include<cstdint>
#include<random>
#include<chrono>
#include<unordered_set>

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

namespace std
{
	template <>
	struct hash<sf::Vector2i>
	{
		std::size_t operator()(const sf::Vector2i& key) const
		{
			using std::size_t;
			using std::hash;
			//cantor pairing function
			return static_cast<std::size_t>(0.5) * (key.x + key.y) * (key.x * key.y + 1) + key.y;
		}
	};
}


class Game
{
public:
	Game();

	void run();
private:
	void handleEvents();
	void update();

	void generate();
	void changeCell(int x, int y, Cell type);
	void floodFill(int x, int y);
	void changeSurrounding(int x, int y, int cellNumber);


	int index(int x, int y);
private:
	sf::Vector2i m_size;
	sf::VertexArray m_cellsVAO;
	sf::RenderWindow m_window;
	sf::Texture m_gameTexture;
	sf::Font m_font;

	std::vector<Cell> m_cells;
	std::vector<Cell> m_playerCells;
	std::unordered_set<sf::Vector2i> m_flagPositions;

	int m_flagCount = 0;
	int m_mineCount = 0;
};