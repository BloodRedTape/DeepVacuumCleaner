#include "map_editor.hpp"
#include "render.hpp"
#include "imgui.h"


MapEditor::MapEditor(sf::Vector2i world_size):
	ZoomMoveApplication(world_size)
{
	//m_Env.LoadFromFile("test.map");
}

void MapEditor::Tick(float dt) {
	Super::Tick(dt);

	if(sf::Keyboard::isKeyPressed(sf::Keyboard::Key::LControl))
		return;

	float forward = 0.f;	
	float rotate = 0.f;

	if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::W)) {
		forward += dt;
	}
	if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::S)) {
		forward -= dt;
	}

	if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::D)) {
		rotate += dt;
	}
	if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::A)) {
		rotate -= dt;
	}

	m_Cleaner.Move(forward * 90, rotate * 60);
}
void MapEditor::OnImGui() {
	Super::OnImGui();

	ImGui::Begin("Map Editor");

	ImGui::Text("Walls: %d", m_Env.Walls.size());
	ImGui::Text("Points: %d", m_Env.Path.size());

	ImGui::End();
}

void MapEditor::Render(sf::RenderTarget& rt) {
	Super::Render(rt);

	m_Env.Draw(rt);


	if (m_WallBegin.has_value()) {
		auto point = sf::Vector2i(m_Window.mapPixelToCoords(sf::Vector2i(MousePosition())));
		Render::DrawLine(sf::Vector2f(m_WallBegin.value()), sf::Vector2f(point), Environment::WallHeight, rt);
	}

	m_Cleaner.Draw(rt);
	m_Cleaner.DrawIntersections(rt, m_Env);
}

void MapEditor::OnEvent(const sf::Event& e){
	Super::OnEvent(e);

	if (const auto* mouse = e.getIf<sf::Event::MouseButtonPressed>()) {
		auto point = sf::Vector2i(m_Window.mapPixelToCoords({mouse->position.x, mouse->position.y}));
		
		if(mouse->button == sf::Mouse::Button::Left){

			if(std::find(m_Env.Path.begin(), m_Env.Path.end(), point) == m_Env.Path.end())
				m_Env.Path.push_back(point);
		}

		if(mouse->button == sf::Mouse::Button::Right){
			m_WallBegin = point;
		}
	}

	if (const auto *mouse = e.getIf<sf::Event::MouseButtonReleased>()){
		auto point = sf::Vector2i(m_Window.mapPixelToCoords({mouse->position.x, mouse->position.y}));

		if(mouse->button == sf::Mouse::Button::Right){
			m_Env.Walls.push_back({m_WallBegin.value(), point});
			m_WallBegin.reset();
		}
	}

	if (const auto *key = e.getIf<sf::Event::KeyPressed>()) {
		if (key->code == sf::Keyboard::Key::S && sf::Keyboard::isKeyPressed(sf::Keyboard::Key::LControl)) {
			OnSave();
		}
	}
}

void MapEditor::OnSave() {
	m_Env.SaveToFile("test.map");
	std::cout << "Saved\n";
}