#include<iostream>
#include<vector>
#include<cstdint>

#include<SFML/Graphics.hpp>

enum class Cell : int8_t
{
	empty = 0,
	mine = 9,
	flag = 10
};

constexpr unsigned width = 1280;
constexpr unsigned height = 720;

void handleEvents(sf::RenderWindow& window);

int main()
{
	sf::RenderWindow window;
	window.create(sf::VideoMode(width, height), "minesweeper", sf::Style::Close);

	while(window.isOpen())
	{
		handleEvents(window);

		window.clear();

		window.display();
	}
}

void handleEvents(sf::RenderWindow & window)
{
	sf::Event e;
	while(window.pollEvent(e))
	{
		switch(e.type)
		{
			case sf::Event::Closed:
			window.close();
			break;

			case sf::Event::KeyReleased:
			if(e.key.code == sf::Keyboard::Escape && e.key.shift)
				window.close();
			break;

			default:
			break;
		}
	}
}
