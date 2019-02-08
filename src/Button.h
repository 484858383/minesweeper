#pragma once

#include<SFML/Graphics.hpp>
#include<functional>


class Button
{
public:
    Button(const std::string& contents,
           const sf::Vector2f& size,
           std::function<void()> func);

    void update(const sf::RenderWindow& window);
    void draw(sf::RenderTarget& render);
	void operator()();

    void setPosition(const sf::Vector2f& pos);
private:
    std::function<void()> m_function;

    sf::Text m_text;

    sf::RectangleShape m_body;
    sf::Vector2f m_size;

	///these should be stored somewhere in only one instance (singleton or something similar), but its like 3 buttons so i think its ok to just allocate on heap here
	sf::Font m_font;
	sf::Texture m_texture;
};
