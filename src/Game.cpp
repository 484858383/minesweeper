#include "Game.h"

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

	if(!m_gameTexture.loadFromFile("res/minesweeper.png"))
		throw std::runtime_error("failed to load textures");

	if(!m_font.loadFromFile("res/pixel.ttf"))
		throw std::runtime_error("failed to load font");

	m_cellsVAO.setPrimitiveType(sf::Quads);

	///temp for now
	m_size = {10, 10};
	m_cells.reserve(m_size.x * m_size.y);
	m_cellsVAO.resize(m_size.x * m_size.y * 4);

	for(int y = 0; y < m_size.y; y++)
	for(int x = 0; x < m_size.x; x++)
	{
		m_cells.push_back(Cell::blank);
		m_playerCells.push_back(Cell::mine);
	}

	generate();
	changeCell(1, 0);
}

void Game::run()
{
	sf::RectangleShape r({200, 200});
	r.setTexture(&m_gameTexture);
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

	if(sf::Mouse::isButtonPressed(sf::Mouse::Left) && c.getElapsedTime().asSeconds() >= 0.00f)
	{
		changeCell(mousePos.x, mousePos.y);
		c.restart();
	}
}

void Game::generate()
{
	static std::vector<sf::Vector2f> quadTemplate = {{0.f, 0.f},{quadSize, 0.f},{quadSize, quadSize},{0.f, quadSize}}; //top left -> bottom left cw
	for(int y = 0; y < m_size.y; y++)
	for(int x = 0; x < m_size.x; x++)
	{
		auto cell = m_cells[index(x, y)];
		int value = static_cast<int>(cell);

		sf::Vector2f position(x, y);

		sf::Vertex* vertex = &m_cellsVAO[index(x, y) * 4];
		for(int i = 0; i < 4; i++)
		{
			vertex[i].position  = (quadSize * position) + quadTemplate[i];
			vertex[i].texCoords.x = quadTexcoords[i].x + value * 16;
			vertex[i].texCoords.y = quadTexcoords[i].y;
		}
	}
}

void Game::changeCell(int x, int y)
{
	static std::vector<sf::Vector2f> quadTemplate = {{0.f, 0.f},{quadSize, 0.f},{quadSize, quadSize},{0.f, quadSize}}; //top left -> bottom left cw

	sf::Vector2f position = {static_cast<float>(x), static_cast<float>(y)};
	int value = static_cast<int>(m_playerCells[index(x, y)]);

	sf::Vertex* vertex = &m_cellsVAO[index(x, y) * 4];
	for(int i = 0; i < 4; i++)
	{
		vertex[i].position = (quadSize * position) + quadTemplate[i];
		vertex[i].texCoords.x = quadTexcoords[i].x + value * 16;
		vertex[i].texCoords.y = quadTexcoords[i].y;
	}
}

int Game::index(int x, int y)
{
	return x + y * m_size.x;
}
