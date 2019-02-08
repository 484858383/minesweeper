#pragma once

#include<vector>
#include<memory>
#include<cstdint>
#include<unordered_set>

#include<SFML/Graphics.hpp>

#include"Button.h"

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
	void changeCell(int x, int y, Cell newCell);
	void floodFill(int x, int y);
	void changeSurrounding(int x, int y, int cellNumber);

	int index(int x, int y);
private:
	sf::Vector2i m_size;
	sf::VertexArray m_cellsVAO;
	sf::RenderWindow m_window;
	sf::Texture m_gameTexture;
	sf::Font m_font;

	sf::Text m_mineText;
	sf::Text m_timeText;
	sf::Clock m_clock;

	std::vector<Cell> m_cells;
	std::vector<Cell> m_playerCells;
	std::unordered_set<sf::Vector2i> m_flagPositions;

	std::vector<std::unique_ptr<Button>> m_buttons;

	float m_quadSize = 72.f;

	int m_difficulty = 0;
	int m_flagCount = 0;
	int m_mineCount = 0;
	int m_mineDensity = 15;

	bool m_loss = false;
	bool m_win = false;
};