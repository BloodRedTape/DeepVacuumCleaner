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
	std::vector<sf::IntRect> ZonesToClean;
	
	GridDecomposition Grid;
	CoverageDecomposition Coverage{Grid};
	Graph CoverageGraph;

	std::size_t CoverageSize = 4;
	sf::Vector2i FrameSize;

	int RenderWallHeight = 15.f;
	static constexpr int NoPath = 0;
	static constexpr int PathWithPoints = 1;
	static constexpr int PathWithLines = 2;
	static constexpr int PathWithColorLines = 3;

	bool IsFullfiled()const;

	void DrawBounds(sf::RenderTarget& rt);

	void Draw(sf::RenderTarget& rt, std::size_t path_drawing_mode = 0, bool draw_zones_to_clean = true);

	void DrawGraph(sf::RenderTarget& rt, bool with_directions, sf::Vector2i world_mouse)const {
#ifndef GRAPH_DEBUG
		CoverageGraph.Draw(rt, Grid.Bounds.getPosition(), with_directions);
#else
		auto visit = Coverage.LocalNearestVisitPointTo(world_mouse - Grid.Bounds.getPosition());

		if (visit.has_value()) {
			CoverageGraph.DrawVertex(rt, visit.value(), Grid.Bounds.getPosition(), with_directions);
		}
#endif
	}

	void DrawZones(sf::RenderTarget &rt, sf::Vector2i mouse_position, bool for_all_cells, bool zone, bool full_zone, bool points, bool cell, bool walls, bool simple);

	void SaveToFile(const std::string& filename);

	void LoadFromFile(const std::string& filename);

	void Bake(std::size_t cell_size, bool optimized_graph);

	sf::IntRect GatherBounds()const;

	sf::Vector2i LocalStartPosition()const {
		return StartPosition - Grid.Bounds.getPosition();
	}

	std::optional<sf::Vector2i> LocalNearestToStartPosition()const {
		return Coverage.LocalNearestVisitPointTo(LocalStartPosition());
	}

	void Clear() {
		Walls.clear();
		Path.clear();
		Grid.Clear();
		ZonesToClean.clear();
		Coverage.Rebuild();
	}
};
