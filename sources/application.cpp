#include "application.hpp"
#include "imgui-SFML.h"
#include "utils/render.hpp"

Application::Application(sf::Vector2i world_size):
	m_WorldSize(world_size)
{}

void Application::Run() {
	m_Window.setVerticalSyncEnabled(false);

	(void)ImGui::SFML::Init(m_Window);

	sf::Clock cl;
	while (m_Window.isOpen()) {
		sf::Time dtt = cl.restart();
		float dt = dtt.asSeconds();


		for (sf::Event e; e = m_Window.pollEvent();) {
			ImGui::SFML::ProcessEvent(m_Window, e);
			OnEvent(e);
		}

		Tick(dt);

		ImGui::SFML::Update(m_Window, dtt);

		OnImGui();

		Render::s_DrawcallsCount = 0;
		m_Window.clear();
		Render(m_Window);
		ImGui::SFML::Render(m_Window);
		m_Window.display();
	}

	ImGui::SFML::Shutdown(m_Window);
}

void Application::Tick(float dt) {	}

void Application::OnEvent(const sf::Event& e) {
	if(e.is<sf::Event::Closed>())
		m_Window.close();
}
void Application::OnImGui() {	}

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

	if (auto wheel = e.getIf<sf::Event::MouseWheelScrolled>()) {
		m_View.zoom(1 - wheel->delta * 0.01);
	}
}
void ZoomMoveApplication::Render(sf::RenderTarget& rt) {
	Super::Render(rt);

	rt.setView(m_View);
}
