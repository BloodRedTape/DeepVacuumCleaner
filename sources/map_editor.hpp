#pragma once

#include <optional>
#include <SFML/System/Vector2.hpp>
#include "env/environment.hpp"
#include "model/vacuum_cleaner.hpp"
#include "application.hpp"
#include "env/path.hpp"

class MapEditor: public ZoomMoveApplication{
	using Super = ZoomMoveApplication;
private:
	std::optional<sf::Vector2i> m_WallBegin;

	Environment m_Env;
	VacuumCleaner m_Cleaner;

	bool m_DrawBounds = false;
	bool m_DrawGridDecomposition = true;
	bool m_DrawNumbers = false;
	
	int m_GridCellSize = 20;
	sf::Vector2i m_StartPosition = {50, 50};

	std::string m_MapFilename;

	std::size_t m_PathDrawingMode = 3;

	int m_SnapGrid = 20;

	bool m_OptimizedGraph = true;

	bool m_CoveragePathDebugging = true;
	bool m_ForAllCells = true;
	bool m_DrawCurrentCell = false;
	bool m_DrawZoneDecomposition = false;
	bool m_DrawFullZoneDecomposition = false;
	bool m_DrawCoveragePoints = false;
	bool m_DrawWallsCoveragePoints = false;
	bool m_DrawCoverageGraph = false;
	bool m_DrawCoverageGraphDirecions = false;

	std::vector<std::unique_ptr<PathBuilder>> m_Builders;
	std::size_t m_Current = 0;
public:

	MapEditor(sf::Vector2i world_size);

	void Tick(float dt)override;

	void OnImGui()override;

	void Render(sf::RenderTarget& rt)override;

	void OnEvent(const sf::Event& e)override;

	void OnSave();

	sf::Vector2i MakeEndPoint()const;

	sf::Vector2i WorldMousePosition()const;

	sf::Vector2i TryWallSnap(sf::Vector2i point)const;

	sf::Vector2i TryGridSnap(sf::Vector2i point)const;
};