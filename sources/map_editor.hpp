#pragma once

#include <optional>
#include <SFML/System/Vector2.hpp>
#include "environment.hpp"
#include "vacuum_cleaner.hpp"
#include "application.hpp"

class MapEditor: public ZoomMoveApplication{
	using Super = ZoomMoveApplication;
private:
	std::optional<sf::Vector2i> m_WallBegin;

	Environment m_Env;
	VacuumCleaner m_Cleaner;

	bool m_DrawBounds = true;
	bool m_DrawGridDecomposition = true;
	bool m_DrawNumbers = false;
	
	int m_Steps = 1;
	sf::Vector2i m_GridCellSize = {20, 20};
	sf::Vector2i m_StartPosition = {50, 50};

	std::string m_MapFilename;

	std::size_t m_PathDrawingMode = 0;

	bool m_CoveragePathDebugging = true;
	bool m_ForAllCells = true;
	bool m_DrawCurrentCell = false;
	bool m_DrawZoneDecomposition = false;
	bool m_DrawFullZoneDecomposition = false;
	bool m_DrawCoveragePoints = false;
public:

	MapEditor(sf::Vector2i world_size);

	void Tick(float dt)override;

	void OnImGui()override;

	void Render(sf::RenderTarget& rt)override;

	void OnEvent(const sf::Event& e)override;

	void OnSave();
};