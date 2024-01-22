#include "application.hpp"

Application::Application(sf::Vector2i world_size):
	m_WorldSize(world_size)
{}

void Application::Run() {
	m_Window.setFramerateLimit(60);

	sf::Clock cl;
	while (m_Window.isOpen()) {
		float dt = cl.restart().asSeconds();

		for (sf::Event e; m_Window.pollEvent(e);) {
			OnEvent(e);
		}

		m_Window.clear();

		Tick(dt);
		
		Render(m_Window);

		m_Window.display();
	}
}

void Application::Tick(float dt) {	}

void Application::OnEvent(const sf::Event& e) { }

void Application::Render(sf::RenderTarget& rt) { }

sf::Vector2f Application::MousePosition()const {
	return sf::Vector2f(sf::Mouse::getPosition(m_Window));
}


ZoomMoveApplication::ZoomMoveApplication(sf::Vector2i size):
	Super(size),
	m_View(m_Window.getView())
{}

void ZoomMoveApplication::Tick(float dt) {
	Super::Tick(dt);

	if (sf::Mouse::isButtonPressed(sf::Mouse::Button::Middle)) {
		if (m_LastMousePosition.has_value()) {
			auto offset = sf::Vector2f(m_LastMousePosition.value() - sf::Mouse::getPosition(m_Window));

			m_View.move(offset);
		}

		m_LastMousePosition = sf::Mouse::getPosition(m_Window);
	} else {
		m_LastMousePosition.reset();
	}
}

void ZoomMoveApplication::OnEvent(const sf::Event& e) { 
	Super::OnEvent(e);

	if (e.type == sf::Event::MouseWheelScrolled) {
		m_View.zoom(1 - e.mouseWheelScroll.delta * 0.01);
	}
}
void ZoomMoveApplication::Render(sf::RenderTarget& rt) {
	Super::Render(rt);

	rt.setView(m_View);
}
