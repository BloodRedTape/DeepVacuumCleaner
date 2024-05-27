#include "map_editor.hpp"
#include "render.hpp"
#include "config.hpp"
#include "imgui.h"


bool InputText(const char* label, std::string& buffer, ImGuiInputTextFlags flags = 0) {
    return ImGui::InputText(label, buffer.data(), buffer.size() + 1, flags | ImGuiInputTextFlags_CallbackResize, [](ImGuiInputTextCallbackData *data)->int {
        if (data->EventFlag == ImGuiInputTextFlags_CallbackResize)
        {
            std::string* str = (std::string*)data->UserData;
            str->resize(data->BufTextLen);
            data->Buf = str->data();
        }
        return 0;
    }, &buffer);
}

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

	ImGui::Checkbox("Draw Bounds", &m_DrawBounds);
	ImGui::Checkbox("Draw Grid Decomposition", &m_DrawGridDecomposition);
	ImGui::Checkbox("Draw Path Numbers", &m_DrawNumbers);
	ImGui::InputInt2("Grid Cell Size", &m_GridCellSize.x);
	ImGui::Text("Vacuum size in cells: %d", (int)VacuumCleaner::Radius * 2 / std::min(m_GridCellSize.x, m_GridCellSize.y));
	ImGui::InputInt("Steps", &m_Steps);

	if (ImGui::Button("Rebuild"))
		m_Env.AutogeneratePath(m_GridCellSize, m_StartPosition, m_Steps);
	
	if(ImGui::Button("Clear"))
		m_Env.Clear();

	InputText("Filename", m_MapFilename);
	if (ImGui::Button("Save")) {
		m_Env.SaveToFile(m_MapFilename + ".map");
	}

	for (auto file : std::filesystem::directory_iterator(".")) {
		if(!file.is_regular_file())
			continue;

		if(file.path().extension() != ".map")
			continue;

		if (ImGui::Button(file.path().string().c_str())) {
			m_Env.LoadFromFile(file.path().string());
			m_MapFilename = file.path().stem().string();
		}
	}

	ImGui::End();
}

void MapEditor::Render(sf::RenderTarget& rt) {
	Super::Render(rt);
	
	if(m_DrawBounds)
		m_Env.DrawBounds(rt);
	if(m_DrawGridDecomposition)
		m_Env.Grid.Draw(rt);
	m_Env.Draw(rt, m_DrawNumbers);


	if (m_WallBegin.has_value()) {
		auto point = sf::Vector2i(m_Window.mapPixelToCoords(sf::Vector2i(MousePosition())));
		Render::DrawLine(sf::Vector2f(m_WallBegin.value()), sf::Vector2f(point), WallHeight, rt);
	}

	m_Cleaner.Draw(rt);
	m_Cleaner.DrawIntersections(rt, m_Env);
}

void MapEditor::OnEvent(const sf::Event& e){
	Super::OnEvent(e);

	if(!ImGui::GetIO().WantCaptureMouse){
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