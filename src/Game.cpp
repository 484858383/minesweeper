#include "Game.h"

#include<iostream>
#include<random>
#include<chrono>

namespace
{
	std::vector<sf::Vector2u> quadTexcoords = {{0, 0}, {16, 0}, {16, 16}, {0, 16}}; //top left -> top right ccw
	constexpr unsigned width = 1280;
	constexpr unsigned height = 720;
}

Game::Game()
{
	m_window.create(sf::VideoMode(width, height), "minesweeper", sf::Style::Close);
	m_window.setFramerateLimit(60);

	if(!m_gameTexture.loadFromFile("res/minesweeper.png"))
		throw std::runtime_error("failed to load textures");

	if(!m_font.loadFromFile("res/pixel.ttf"))
		throw std::runtime_error("failed to load font");

	m_mineText.setFont(m_font);
	m_mineText.setString("mines: ");
	m_mineText.setCharacterSize(48);
	m_mineText.setOutlineThickness(2.f);
	m_mineText.setFillColor(sf::Color::White);
	m_mineText.setOutlineColor(sf::Color::Black);
	m_mineText.setPosition(728, 32);

	m_timeText.setFont(m_font);
	m_timeText.setString("time: ");
	m_timeText.setCharacterSize(48);
	m_timeText.setOutlineThickness(2.f);
	m_timeText.setFillColor(sf::Color::White);
	m_timeText.setOutlineColor(sf::Color::Black);
	m_timeText.setPosition(728, 0);

	m_cellsVAO.setPrimitiveType(sf::Quads);

	//allocate enough memory for largest size
	m_cells.reserve(30 * 24);
	m_playerCells.reserve(30 * 24);

	m_buttons.emplace_back(std::make_unique<Button>("Beginner", sf::Vector2f(128, 32), [&]() {m_size = {10, 10}; m_quadSize = 72.f; m_mineDensity = 13; m_difficulty = 0; generate(); }));
	m_buttons.back()->setPosition({728, 672});

	m_buttons.emplace_back(std::make_unique<Button>("Intermediate", sf::Vector2f(128, 32), [&]() {m_size = {16, 16}; m_quadSize = 45.f; m_mineDensity = 50; m_difficulty = 1; generate(); }));
	m_buttons.back()->setPosition({860, 672});

	m_buttons.emplace_back(std::make_unique<Button>("Expert", sf::Vector2f(128, 32), [&]() {m_size = {24, 24}; m_quadSize = 30.f; m_mineDensity = 100; m_difficulty = 2; generate(); }));
	m_buttons.back()->setPosition({992, 672});

	(*m_buttons[m_difficulty])(); //dereference ->> call () operator
}

void Game::run()
{
	sf::RectangleShape bg({558, 716});
	bg.setPosition(720, 2);
	bg.setFillColor(sf::Color(220, 220, 220, 255));
	bg.setOutlineColor(sf::Color::Black);
	bg.setOutlineThickness(2.f);

	bool stop = false;
	while(m_window.isOpen())
	{
		handleEvents();

		if(sf::Keyboard::isKeyPressed(sf::Keyboard::R))
		{
			(*m_buttons[m_difficulty])();
			stop = false;
		}

		if(!m_loss && !m_win)
			update();

		if((m_loss || m_win) && !stop)
		{
			for(int y = 0; y < m_size.y; y++)
			for(int x = 0; x < m_size.x; x++)
				changeCell(x, y, m_cells[index(x, y)]);
			stop = true;
			std::cout << (m_win ? "you win" : "you lose") << ", press R to play again\n";
		}

		for(auto& button : m_buttons)
			button->update(m_window);

		m_window.clear();
		m_window.draw(m_cellsVAO, &m_gameTexture);
		m_window.draw(bg);
		
		for(auto& button : m_buttons)
			button->draw(m_window);

		m_window.draw(m_mineText);
		m_window.draw(m_timeText);
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
	mousePos.x /= m_quadSize;
	mousePos.y /= m_quadSize;

	mousePos.x = std::max(std::min(mousePos.x, m_size.x), 0);
	mousePos.y = std::max(std::min(mousePos.y, m_size.y -1), 0);

	if(mousePos.x >= m_size.x)
		return;

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
			m_loss = true;

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

		if(playerCell == Cell::flag)
			changeCell(mousePos.x, mousePos.y, Cell::blank);
		else if(playerCell == Cell::blank)
			changeCell(mousePos.x, mousePos.y, Cell::flag);
	}
	
	if(m_flagCount == m_mineCount)
	{
		for(auto& position : m_flagPositions)
			if(m_cells[index(position.x, position.y)] != Cell::mine)
				return;
		m_win = true;
	}

	m_mineText.setString("mines: " + std::to_string(m_mineCount >= m_flagCount ? m_mineCount - m_flagCount : 0));
	m_timeText.setString("time: " + std::to_string(static_cast<int>(m_clock.getElapsedTime().asSeconds())));
}

void Game::generate()
{
	std::mt19937 rng;
	rng.seed(std::time(nullptr));
	std::uniform_int_distribution<> xRand(0, m_size.x -1);
	std::uniform_int_distribution<> yRand(0, m_size.y -1);

	m_cells.clear();
	m_playerCells.clear();
	m_cellsVAO.clear();

	m_win = false;
	m_loss = false;
	m_flagCount = 0;
	m_mineCount = 0;
	m_clock.restart();
	m_flagPositions.clear();

	m_cellsVAO.resize(m_size.x * m_size.y * 4);

	for(int y = 0; y < m_size.y; y++)
	for(int x = 0; x < m_size.x; x++)
	{
		m_cells.push_back(Cell::empty);
		m_playerCells.push_back(Cell::blank);
	}

	//add mines
	while(m_mineCount < m_mineDensity)
	{
		int x = xRand(rng);
		int y = yRand(rng);
		auto& cell = m_cells[index(x, y)];

		if(cell == Cell::empty)
		{
			++m_mineCount;
			cell = Cell::mine;
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
	std::vector<sf::Vector2f> quadTemplate = {{0.f, 0.f},{m_quadSize, 0.f},{m_quadSize, m_quadSize},{0.f, m_quadSize}}; //top left -> bottom left cw
	for(int y = 0; y < m_size.y; y++)
	for(int x = 0; x < m_size.x; x++)
	{
		int textureOffset = static_cast<int>(Cell::blank);
		sf::Vector2f position(x, y);

		sf::Vertex* vertex = &m_cellsVAO[index(x, y) * 4];
		for(int i = 0; i < 4; i++)
		{
			vertex[i].position  = (m_quadSize * position) + quadTemplate[i];
			vertex[i].texCoords.x = quadTexcoords[i].x + textureOffset * 16;
			vertex[i].texCoords.y = quadTexcoords[i].y;
		}
	}
}

void Game::changeCell(int x, int y, Cell newCell)
{
	std::vector<sf::Vector2f> quadTemplate = {{0.f, 0.f},{m_quadSize, 0.f},{m_quadSize, m_quadSize},{0.f, m_quadSize}}; //top left -> bottom left cw

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
		vertex[i].position = (m_quadSize * position) + quadTemplate[i];
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
			m_loss = true;
			return;
		}

		if(playerCell == Cell::flag)
			flagCount++;
	}

	if(flagCount != cellNumber)
	{
		m_loss = true;
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
