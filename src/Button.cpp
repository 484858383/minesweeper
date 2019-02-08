#include"Button.h"

Button::Button(const std::string& contents,
               const sf::Vector2f& size,
               std::function<void()>func)

:m_function(func),m_size(size)
{
	if(!m_texture.loadFromFile("res/button.png"))
		throw std::runtime_error("failed to load button texture");

	if(!m_font.loadFromFile("res/pixel.ttf"))
		throw std::runtime_error("failed to load font");

	m_text.setFont(m_font);
    m_text.setString(contents);
    m_text.setCharacterSize(contents.size() >= 10 ? size.y / 1.1 : size.y);
    m_text.setFillColor(sf::Color::White);

    m_body.setSize(m_size);
	m_body.setTexture(&m_texture);
}

void Button::update(const sf::RenderWindow& window)
{
    static sf::Clock timer;
    bool highlighted = false;

    auto mPos = window.mapPixelToCoords(sf::Mouse::getPosition(window));

    if(m_body.getGlobalBounds().contains(mPos))
    {
        highlighted = true;
        if(timer.getElapsedTime().asSeconds() > 0.1f)
        {
            timer.restart();
            if(sf::Mouse::isButtonPressed(sf::Mouse::Left))
                m_function();
        }
    }
    if(highlighted)
        m_text.setFillColor(sf::Color::Red);
    else
        m_text.setFillColor(sf::Color::White);
}

void Button::draw(sf::RenderTarget& render)
{
    render.draw(m_body);
    render.draw(m_text);
}

void Button::setPosition(const sf::Vector2f& pos)
{
    float ratio = m_size.x / m_size.y;
    m_body.setPosition(pos);
    m_text.setPosition(pos.x + m_size.x / ratio, pos.y - m_size.y / 4.0f);
	if(m_text.getString().getSize() >= 10)
		m_text.move(-20, 0);
}
