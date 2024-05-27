#pragma once

#include <SFML/Graphics.hpp>

class Application {
protected:
	sf::Vector2i m_WorldSize{1920, 1080};
	sf::RenderWindow m_Window{sf::VideoMode(sf::Vector2u(m_WorldSize)), "Vacuum cleaner"};
public:

	Application(sf::Vector2i world_size);

	void Run();

	virtual void Tick(float dt);

	virtual void OnEvent(const sf::Event& e);

	virtual void OnImGui();

	virtual void Render(sf::RenderTarget& rt);

	sf::Vector2f MousePosition()const;
};

class ZoomMoveApplication : public Application {
	using Super = Application;
protected:
	sf::View m_View;

	std::optional<sf::Vector2i> m_LastMousePosition;
public:
	ZoomMoveApplication(sf::Vector2i size);

	virtual void Tick(float dt);

	virtual void OnEvent(const sf::Event& e) override;

	virtual void Render(sf::RenderTarget& rt) override;
};
