#include "Game.h"

#include<iostream>
#include<random>
#include<chrono>

namespace
{
	std::vector<sf::Vector2u> quadTexcoords = {{0, 0}, {16, 0}, {16, 16}, {0, 16}}; //top left -> top right ccw
	constexpr unsigned width = 1280;
	constexpr unsigned height = 720;
	constexpr float quadSize = 64.f;
}

Game::Game()
{
	m_window.create(sf::VideoMode(width, height), "minesweeper", sf::Style::Close);
	m_window.setFramerateLimit(60);

	if(!m_gameTexture.loadFromFile("res/minesweeper.png"))
		throw std::runtime_error("failed to load textures");

	if(!m_font.loadFromFile("res/pixel.ttf"))
		throw std::runtime_error("failed to load font");

	m_cellsVAO.setPrimitiveType(sf::Quads);

	///temp for now
	m_size = {10, 10};
	m_cells.reserve(m_size.x * m_size.y);
	m_playerCells.reserve(m_size.x * m_size.y);
	m_cellsVAO.resize(m_size.x * m_size.y * 4);

	for(int y = 0; y < m_size.y; y++)
	for(int x = 0; x < m_size.x; x++)
	{
		m_cells.push_back(Cell::empty);
		m_playerCells.push_back(Cell::blank);
	}

	generate();
}

void Game::run()
{
	while(m_window.isOpen())
	{
		handleEvents();

		m_window.clear();

		update();
		m_window.draw(m_cellsVAO, &m_gameTexture);

		m_window.display();
	}
}

void Game::handleEvents()
{
	sf::Event e;
	while(m_window.pollEvent(e))
	switch(e.type)
	{
		case sf::Event::Closed:
		m_window.close();
		break;

		case sf::Event::KeyReleased:
		if(e.key.code == sf::Keyboard::Escape && e.key.shift)
			m_window.close();
		break;

		default:
		break;
	}
}

void Game::update()
{
	static sf::Clock c;
	auto mousePos = sf::Mouse::getPosition(m_window);
	mousePos.x /= quadSize;
	mousePos.y /= quadSize;

	mousePos.x = std::max(std::min(mousePos.x, m_size.x -1), 0);
	mousePos.y = std::max(std::min(mousePos.y, m_size.y -1), 0);

	if(sf::Mouse::isButtonPressed(sf::Mouse::Left) && c.getElapsedTime().asSeconds() >= 0.15f)
	{
		c.restart();
		auto currentCell = m_cells[index(mousePos.x, mousePos.y)];
		auto playerCell  = m_playerCells[index(mousePos.x, mousePos.y)];

		if(playerCell >= Cell::one && playerCell <= Cell::seven && sf::Keyboard::isKeyPressed(sf::Keyboard::LShift))
		{
			changeSurrounding(mousePos.x, mousePos.y, static_cast<int>(playerCell));
			return;
		}

		if(currentCell == Cell::mine)
			std::cout << "loss\n";

		if(currentCell == Cell::empty)
			floodFill(mousePos.x, mousePos.y);
		else
			changeCell(mousePos.x, mousePos.y, playerCell == Cell::flag ? Cell::blank : currentCell);
	}
	if(sf::Mouse::isButtonPressed(sf::Mouse::Right) && c.getElapsedTime().asSeconds() >= 0.15f)
	{
		c.restart();
		auto currentCell = m_cells[index(mousePos.x, mousePos.y)];
		auto playerCell = m_playerCells[index(mousePos.x, mousePos.y)];

		changeCell(mousePos.x, mousePos.y, playerCell == Cell::flag ? Cell::blank : Cell::flag);
	}
	
	if(m_flagCount == m_mineCount)
	{
		for(auto& position : m_flagPositions)
			if(m_cells[index(position.x, position.y)] != Cell::mine)
				return;
		std::cout << "win!";
	}

}

void Game::generate()
{
	std::mt19937 rng;
	rng.seed(std::time(nullptr));
	std::uniform_int_distribution<> dist(0, 9);

	//add mines
	for(auto& cell : m_cells)
	{
		if(dist(rng) == 0)
		{
			cell = Cell::mine;
			m_mineCount++;
		}
	}
		
	//add numbers
	for(int y = 0; y < m_size.y; y++)
	for(int x = 0; x < m_size.x; x++)
	{
		if(m_cells[index(x, y)] == Cell::mine)
			continue;

		//for each neighbour
		int mineCount = 0;
		for(int j = -1; j < 2; j++)
		for(int i = -1; i < 2; i++)
		{
			int Y = j + y;
			if(Y < 0 || Y >= m_size.y)
				continue;

			int X = i + x;
			if(X < 0 || X >= m_size.x)
				continue;

			if(i == 0 && j == 0)
				continue;

			if(m_cells[index(X, Y)] == Cell::mine)
				++mineCount;
		}

		m_cells[index(x, y)] = static_cast<Cell>(mineCount);
	}

	//texture
	static std::vector<sf::Vector2f> quadTemplate = {{0.f, 0.f},{quadSize, 0.f},{quadSize, quadSize},{0.f, quadSize}}; //top left -> bottom left cw
	for(int y = 0; y < m_size.y; y++)
	for(int x = 0; x < m_size.x; x++)
	{
		int textureOffset = static_cast<int>(Cell::blank);
		sf::Vector2f position(x, y);

		sf::Vertex* vertex = &m_cellsVAO[index(x, y) * 4];
		for(int i = 0; i < 4; i++)
		{
			vertex[i].position  = (quadSize * position) + quadTemplate[i];
			vertex[i].texCoords.x = quadTexcoords[i].x + textureOffset * 16;
			vertex[i].texCoords.y = quadTexcoords[i].y;
		}
	}
}

void Game::changeCell(int x, int y, Cell newCell)
{
	static std::vector<sf::Vector2f> quadTemplate = {{0.f, 0.f},{quadSize, 0.f},{quadSize, quadSize},{0.f, quadSize}}; //top left -> bottom left cw

	if(newCell == Cell::flag && m_playerCells[index(x, y)] != Cell::flag)
	{
		m_flagCount++;
		m_flagPositions.insert({x, y});
	}
		
	if(m_playerCells[index(x, y)] == Cell::flag && newCell != Cell::flag)
	{
		m_flagCount--;
		m_flagPositions.erase(m_flagPositions.find({x, y}));
	}

	sf::Vector2f position(x, y);

	int textureOffset = static_cast<int>(newCell);
	m_playerCells[index(x, y)] = newCell;

	sf::Vertex* vertex = &m_cellsVAO[index(x, y) * 4];
	for(int i = 0; i < 4; i++)
	{
		vertex[i].position = (quadSize * position) + quadTemplate[i];
		vertex[i].texCoords.x = quadTexcoords[i].x + textureOffset * 16;
		vertex[i].texCoords.y = quadTexcoords[i].y;
	}
}

void Game::floodFill(int x, int y)
{
	auto cell = m_cells[index(x, y)];
	if(cell != Cell::mine && m_playerCells[index(x, y)] == Cell::blank)
	{
		changeCell(x, y, cell);

		//for each neighbour
		for(int j = -1; j < 2; j++)
		for(int i = -1; i < 2; i++)
		{
			int Y = j + y;
			if(Y < 0 || Y >= m_size.y)
				continue;

			int X = i + x;
			if(X < 0 || X >= m_size.x)
				continue;

			if(i == 0 && j == 0)
				continue;
			if(cell != Cell::empty)
				return;

			floodFill(X, Y);
		}
	}
}

void Game::changeSurrounding(int x, int y, int cellNumber)
{
	///clears all neighbouring cells when all correct flags are placed,
	///have to count up surrounding flags to check for mistakes first
	//for each neighbour
	int flagCount = 0;
	for(int j = -1; j < 2; j++)
	for(int i = -1; i < 2; i++)
	{
		int Y = j + y;
		if(Y < 0 || Y >= m_size.y)
			continue;

		int X = i + x;
		if(X < 0 || X >= m_size.x)
			continue;

		if(i == 0 && j == 0)
			continue;

		auto cell = m_cells[index(X, Y)];
		auto playerCell = m_playerCells[index(X, Y)];

		if(cell == Cell::mine && playerCell != Cell::flag)
		{
			std::cout << "loss\n";
			return;
		}

		if(playerCell == Cell::flag)
			flagCount++;
	}

	if(flagCount != cellNumber)
	{
		std::cout << "loss\n";
		return;
	}

	//for each neighbour
	for(int j = -1; j < 2; j++)
	for(int i = -1; i < 2; i++)
	{
		int Y = j + y;
		if(Y < 0 || Y >= m_size.y)
			continue;

		int X = i + x;
		if(X < 0 || X >= m_size.x)
			continue;

		if(i == 0 && j == 0)
			continue;

		auto cell = m_cells[index(X, Y)];
		auto playerCell = m_playerCells[index(X, Y)];

		if(playerCell == Cell::flag)
			continue;

		if(cell == Cell::empty)
			floodFill(X, Y);
		else
			changeCell(X, Y, cell);
	}
}

int Game::index(int x, int y)
{
	return x + y * m_size.x;
}
