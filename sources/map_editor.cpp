#include "map_editor.hpp"
#include "utils/render.hpp"
#include "config.hpp"
#include "utils/imgui.hpp"
#include "utils/math.hpp"

#define EDITOR_WITH_PATH 0

MapEditor::MapEditor(sf::Vector2i world_size):
	ZoomMoveApplication(world_size)
{
	//m_Env.LoadFromFile("test.map");
	m_Builders.push_back(std::make_unique<BreadthSearchPathFinder>());
	m_Builders.push_back(std::make_unique<BreadthSearchWithSortPathFinder>());
	m_Builders.push_back(std::make_unique<FirstNearWallPathBuilder>());
	m_Builders.push_back(std::make_unique<DirectionSortPathBuilder>());
	m_Builders.push_back(std::make_unique<RightFirstPathBuilder>());
	//m_Builders.push_back(std::make_unique<RightFirstForWallsPathBuilder>());
	m_Builders.push_back(std::make_unique<NonOccupiedPathBuilder>());

	m_Current = m_Builders.size() - 1;
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
	ImGui::InputInt("SnapGrid", &m_SnapGrid);
	ImGui::InputInt("WallThickness", &m_Env.RenderWallHeight);
	ImGui::Separator();

	ImGui::Text("Wall align key: %s", sf::Keyboard::getDescription(sf::Keyboard::Scancode::LShift).toAnsiString().c_str());

	ImGui::Text("Walls: %d", m_Env.Walls.size());
	ImGui::Text("Points: %d", m_Env.Path.size());
	ImGui::Text("CleanZones: %d", m_Env.ZonesToClean.size());
	ImGui::Text("GraphVertices: %d", m_Env.CoverageGraph.Size());
	if(m_Env.Grid.Bounds.getSize().x){
		auto nearest = m_Env.LocalNearestToStartPosition();
		if(nearest.has_value()){
			int reachable = m_Env.CoverageGraph.CountReachableFrom(nearest.value());
			ImGui::Text("GraphReachableFromStart: %d", reachable);
			ImGui::Text("Map Efficiency: %.1f%%", (reachable / float(m_Env.Path.size())) * 100);
		}
	}

	ImGui::SimpleCombo("EditTool", (std::size_t*)&m_Tool, EditToolMembers());

	if(ImGui::Button("Clear Zones"))
		m_Env.ZonesToClean.clear();

	if(ImGui::Button("Clear"))
		m_Env.Clear();


	ImGui::InputText("Filename", m_MapFilename);
	if (ImGui::Button("Save")) {
		m_Env.SaveToFile(m_MapFilename + ".map");
	}

	for (auto file : std::filesystem::directory_iterator(".")) {
		if(!file.is_regular_file())
			continue;

		if(file.path().extension() != ".map")
			continue;

		if (ImGui::Button(file.path().string().c_str())) {
			m_Env.Clear();
			m_Env.LoadFromFile(file.path().string());
			m_MapFilename = file.path().stem().string();
		}
	}
	ImGui::Separator();

	ImGui::SimpleCombo("Path Drawing mode", &m_PathDrawingMode, {"None", "Points", "Line", "ColorLine"});
	ImGui::Checkbox("Draw Bounds", &m_DrawBounds);
	ImGui::Checkbox("Draw Clean Zones", &m_DrawCleanZones);
	ImGui::Checkbox("Draw Grid Decomposition", &m_DrawGridDecomposition);
	ImGui::InputInt("Grid Cell Size", &m_GridCellSize);
	
	ImGui::Spacing();
	ImGui::Checkbox("Optimized Graph", &m_OptimizedGraph);
	if (ImGui::Button("Bake"))
		m_Env.Bake(m_GridCellSize, m_OptimizedGraph);
	
	std::vector<std::string> names;
	for(const auto &builder: m_Builders)
		names.push_back(builder->Name());

	ImGui::SimpleCombo("Path Builder", &m_Current, names);
	if(ImGui::Button("Build Path")){
		m_Env.Path = m_Builders[m_Current]->MakePath(m_Env);
		for(auto &point: m_Env.Path)
			point += m_Env.Grid.Bounds.getPosition();
	}

	ImGui::Separator();

	ImGui::Checkbox("Coverage path planning debug", &m_CoveragePathDebugging);

	if (m_CoveragePathDebugging) {
		ImGui::SimpleCombo("Draw About", &m_ForAllCells, "Whole Map", "Cell under cursor");
		ImGui::Checkbox("DrawCurrentCell", &m_DrawCurrentCell);
		ImGui::Checkbox("DrawZoneDecomposition", &m_DrawZoneDecomposition);
		ImGui::Checkbox("DrawFullZoneDecomposition", &m_DrawFullZoneDecomposition);
		ImGui::Checkbox("DrawCoveragePoints", &m_DrawCoveragePoints);
		ImGui::Checkbox("DrawWallsCoveragePoints", &m_DrawWallsCoveragePoints);
		ImGui::Checkbox("DrawCoverageGraph", &m_DrawCoverageGraph);
		ImGui::Checkbox("DrawCoverageGraphDirections", &m_DrawCoverageGraphDirecions);
		ImGui::Checkbox("DrawSimpleZoneDecomposition", &m_DrawSimpleZoneDecomposition);
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
		m_CoveragePathDebugging && m_DrawCurrentCell,
		m_CoveragePathDebugging && m_DrawWallsCoveragePoints,
		m_CoveragePathDebugging && m_DrawSimpleZoneDecomposition 
	);

	if(m_CoveragePathDebugging && m_DrawCoverageGraph)
		m_Env.DrawGraph(rt, m_DrawCoverageGraphDirecions, WorldMousePosition());
	m_Env.Draw(rt, m_PathDrawingMode, m_DrawCleanZones);

	
	if(m_Tool == EditTool::Wall){
		if (m_ToolCache.has_value()) {
			Render::DrawLine(rt, m_ToolCache.value(), MakeEndPoint(), m_Env.RenderWallHeight);
		}
	}
	if(m_Tool == EditTool::Zone){
		if (m_ToolCache.has_value()) {
			Render::DrawRect(rt, Math::MakeRect(m_ToolCache.value(), MakeEndPoint()), sf::Color::Yellow * sf::Color(255, 255, 255, 40), 4, sf::Color::Yellow);
		}
	}

	m_Cleaner.Draw(rt);
	m_Cleaner.DrawIntersections(rt, m_Env);
}

void MapEditor::OnEvent(const sf::Event& e){
	Super::OnEvent(e);

	if(!ImGui::GetIO().WantCaptureMouse){
		if (const auto* mouse = e.getIf<sf::Event::MouseButtonPressed>()) {
			auto point = WorldMousePosition();

			if(mouse->button == sf::Mouse::Button::Right){
				if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::LAlt))
					point = TryGridSnap(point);
				if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::LControl))
					point = TryWallSnap(point);

				m_ToolCache = point;
			}

			
			if(m_Tool == EditTool::Start){
				m_Env.StartPosition = WorldMousePosition();
			}
		}

		if (const auto *mouse = e.getIf<sf::Event::MouseButtonReleased>()){
			if(m_Tool == EditTool::Wall){
				if(mouse->button == sf::Mouse::Button::Right && m_ToolCache.has_value()){
					m_Env.Walls.push_back({m_ToolCache.value(), MakeEndPoint()});
				}
			}

			if (m_Tool == EditTool::Zone) {
				if(mouse->button == sf::Mouse::Button::Right && m_ToolCache.has_value()){
					m_Env.ZonesToClean.push_back(Math::MakeRect(m_ToolCache.value(), MakeEndPoint()));
				}

			}
			m_ToolCache.reset();
		}
	}

	if (const auto *key = e.getIf<sf::Event::KeyPressed>()) {
		if (key->code == sf::Keyboard::Key::S && sf::Keyboard::isKeyPressed(sf::Keyboard::Key::LControl)) {
			OnSave();
		}
#if !EDITOR_WITH_PATH
		if (key->code == sf::Keyboard::Key::Z && sf::Keyboard::isKeyPressed(sf::Keyboard::Key::LControl)) {
			m_Env.Walls.pop_back();
		}
#endif
	}
}

void MapEditor::OnSave() {
	m_Env.SaveToFile("test.map");
	std::cout << "Saved\n";
}

sf::Vector2i MapEditor::MakeEndPoint() const{
	sf::Vector2i point = WorldMousePosition();

	if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::LShift)) {
		sf::Vector2i direction = (point - m_ToolCache.value());

		if(std::abs(direction.x) < std::abs(direction.y))
			point.x = m_ToolCache.value().x;
		else
			point.y = m_ToolCache.value().y;
	}

	if(sf::Keyboard::isKeyPressed(sf::Keyboard::Key::LAlt))
		point = TryGridSnap(point);

	if(sf::Keyboard::isKeyPressed(sf::Keyboard::Key::LControl))
		point = TryWallSnap(point);

	return point;
}

sf::Vector2i MapEditor::WorldMousePosition() const{
	return sf::Vector2i(m_Window.mapPixelToCoords(sf::Vector2i(MousePosition())));
}

sf::Vector2i MapEditor::TryWallSnap(sf::Vector2i point) const{
	const float SnapDistance = 20.f;

	for (auto wall : m_Env.Walls) {
		if(sf::Vector2f(wall.Start - point).length() <= SnapDistance)
			point = wall.Start;
		if(sf::Vector2f(wall.End - point).length() <= SnapDistance)
			point = wall.End;
	}

	return point;
}

sf::Vector2i MapEditor::TryGridSnap(sf::Vector2i point) const{

	point.x = (point.x / m_SnapGrid) * m_SnapGrid + m_SnapGrid / 2;
	point.y = (point.y / m_SnapGrid) * m_SnapGrid + m_SnapGrid / 2;

	return point;
}

