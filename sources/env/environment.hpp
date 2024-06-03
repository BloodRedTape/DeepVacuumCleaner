#pragma once

#include <optional>
#include <vector>
#include <SFML/System/Vector2.hpp>
#include <SFML/Graphics/RenderTarget.hpp>
#include <ostream>
#include <istream>
#include "env/grid.hpp"
#include "env/coverage.hpp"
#include "env/graph.hpp"
#include "config.hpp"

struct Environment {
	std::vector<sf::Vector2i> Path;
	std::vector<Wall> Walls;
	sf::Vector2i StartPosition;
	
	GridDecomposition Grid;
	CoverageDecomposition Coverage{Grid};
	Graph CoverageGraph;

	std::size_t CoverageSize = 4;
	sf::Vector2i FrameSize;

	int RenderWallHeight = 15.f;
	static constexpr int NoPath = 0;
	static constexpr int PathWithPoints = 1;
	static constexpr int PathWithLines = 2;

	bool IsFullfiled()const;

	void DrawBounds(sf::RenderTarget& rt);

	void Draw(sf::RenderTarget& rt, std::size_t path_drawing_mode = 0);

	void DrawGraph(sf::RenderTarget& rt)const {
		CoverageGraph.Draw(rt, Grid.Bounds.getPosition());
	}

	void DrawZones(sf::RenderTarget &rt, sf::Vector2i mouse_position, bool for_all_cells, bool zone, bool full_zone, bool points, bool cell, bool walls);

	void SaveToFile(const std::string& filename);

	void LoadFromFile(const std::string& filename);

	void AutogeneratePath(std::size_t cell_size, sf::Vector2i start_position, bool build_path);

	sf::IntRect GatherBounds()const;

	void Clear() {
		Walls.clear();
		Path.clear();
		Grid.Clear();
		Coverage.Rebuild();
	}
};
