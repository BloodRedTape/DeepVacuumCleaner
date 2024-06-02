#include "map_editor.hpp"
#include "render.hpp"
#include "config.hpp"
#include "imgui.h"
#include "imgui.hpp"

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


	ImGui::Text("Framerate: %f", ImGui::GetIO().Framerate);
	ImGui::Text("DrawCalls: %d", (int)Render::s_DrawcallsCount);
	ImGui::Separator();

	ImGui::Text("Wall align key: %s", sf::Keyboard::getDescription(sf::Keyboard::Scancode::LShift).toAnsiString().c_str());

	ImGui::Text("Walls: %d", m_Env.Walls.size());
	ImGui::Text("Points: %d", m_Env.Path.size());

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
	ImGui::Separator();

	ImGui::SimpleCombo("Path Drawing mode", &m_PathDrawingMode, {"None", "Points", "Line"});
	ImGui::Checkbox("Draw Bounds", &m_DrawBounds);
	ImGui::Checkbox("Draw Grid Decomposition", &m_DrawGridDecomposition);
	ImGui::InputInt2("Grid Cell Size", &m_GridCellSize.x);
	ImGui::Text("Vacuum size in cells: %d", (int)CleanerRadius * 2 / std::min(m_GridCellSize.x, m_GridCellSize.y));
	ImGui::InputInt("Steps", &m_Steps);

	if (ImGui::Button("Rebuild"))
		m_Env.AutogeneratePath(m_GridCellSize, m_StartPosition, m_Steps);

	ImGui::Separator();

	ImGui::Checkbox("Coverage path planning debug", &m_CoveragePathDebugging);

	if (m_CoveragePathDebugging) {
		ImGui::SimpleCombo("Draw About", &m_ForAllCells, "Whole Map", "Cell under cursor");
		ImGui::Checkbox("ForAllCells", &m_ForAllCells);
		ImGui::Checkbox("DrawCurrentCell", &m_DrawCurrentCell);
		ImGui::Checkbox("DrawZoneDecomposition", &m_DrawZoneDecomposition);
		ImGui::Checkbox("DrawFullZoneDecomposition", &m_DrawFullZoneDecomposition);
		ImGui::Checkbox("DrawCoveragePoints", &m_DrawCoveragePoints);
	}

	ImGui::End();
}

void MapEditor::Render(sf::RenderTarget& rt) {
	Super::Render(rt);
	
	if(m_DrawBounds)
		m_Env.DrawBounds(rt);
	if(m_DrawGridDecomposition)
		m_Env.Grid.Draw(rt);

	m_Env.DrawZones(
		rt, 
		WorldMousePosition(),
		m_CoveragePathDebugging && m_ForAllCells,
		m_CoveragePathDebugging && m_DrawZoneDecomposition, 
		m_CoveragePathDebugging && m_DrawFullZoneDecomposition, 
		m_CoveragePathDebugging && m_DrawCoveragePoints, 
		m_CoveragePathDebugging && m_DrawCurrentCell
	);
	m_Env.Draw(rt, m_PathDrawingMode);


	if (m_WallBegin.has_value()) {
		Render::DrawLine(rt, m_WallBegin.value(), MakeEndPoint(), WallHeight);
	}

	m_Cleaner.Draw(rt);
	m_Cleaner.DrawIntersections(rt, m_Env);
}

void MapEditor::OnEvent(const sf::Event& e){
	Super::OnEvent(e);

	if(!ImGui::GetIO().WantCaptureMouse){
		if (const auto* mouse = e.getIf<sf::Event::MouseButtonPressed>()) {
			auto point = WorldMousePosition();
		
			if(mouse->button == sf::Mouse::Button::Left){

				if(std::find(m_Env.Path.begin(), m_Env.Path.end(), point) == m_Env.Path.end())
					m_Env.Path.push_back(point);
			}

			if(mouse->button == sf::Mouse::Button::Right){
				if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::LControl))
					point = TrySnap(point);

				m_WallBegin = point;
			}
		}

		if (const auto *mouse = e.getIf<sf::Event::MouseButtonReleased>()){
			if(mouse->button == sf::Mouse::Button::Right && m_WallBegin.has_value()){
				m_Env.Walls.push_back({m_WallBegin.value(), MakeEndPoint()});
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

sf::Vector2i MapEditor::MakeEndPoint() const{
	sf::Vector2i point = WorldMousePosition();

	if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::LShift)) {
		sf::Vector2i direction = (point - m_WallBegin.value());

		if(std::abs(direction.x) < std::abs(direction.y))
			point.x = m_WallBegin.value().x;
		else
			point.y = m_WallBegin.value().y;
	}

	if(sf::Keyboard::isKeyPressed(sf::Keyboard::Key::LControl))
		point = TrySnap(point);

	return point;
}

sf::Vector2i MapEditor::WorldMousePosition() const{
	return sf::Vector2i(m_Window.mapPixelToCoords(sf::Vector2i(MousePosition())));
}

sf::Vector2i MapEditor::TrySnap(sf::Vector2i point) const{
	const float SnapDistance = 20.f;

	for (auto wall : m_Env.Walls) {
		if(sf::Vector2f(wall.Start - point).length() <= SnapDistance)
			point = wall.Start;
		if(sf::Vector2f(wall.End - point).length() <= SnapDistance)
			point = wall.End;
	}

	return point;
}
