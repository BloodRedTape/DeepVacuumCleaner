#include "graph.hpp"
#include "utils/render.hpp"

void Graph::Draw(sf::RenderTarget& rt, sf::Vector2i offset)const {

	for (const auto& [vertex, neighbours] : m_Vertices) {
		for(const auto &neighbour: neighbours.Neighbours){
			Render::DrawLine(rt, vertex + offset, neighbour + offset, 2.f, sf::Color::White);
		}
	}

	for (const auto& [vertex, neighbours] : m_Vertices) {
		Render::DrawCircle(rt, vertex + offset, 5.f, sf::Color::Green);
	}
}



Graph Graph::MakeFrom(const CoverageDecomposition& coverage_grid) {
	std::unordered_map<sf::Vector2i, Neighbours> graph;

	coverage_grid.ForEachCoverage([&](sf::Vector2i coverage) {
		auto points = coverage_grid.LocatedVisitPointsCache[coverage];
		auto neighbours = coverage_grid.GatherNeighboursVisitPoints(coverage);

		for (auto point : points) {
			auto &neighbour_points = graph[point].Neighbours;
			std::copy(points.begin(), points.end(), std::back_inserter(neighbour_points));
	
			//remove ourself
			std::remove(neighbour_points.begin(), neighbour_points.end(), point);
			neighbour_points.pop_back();
			
			for (auto neighbour : neighbours) {
				if(coverage_grid.AreDirectlyReachable(point, neighbour))
					neighbour_points.push_back(neighbour);
			}
		}
	});


	return {std::move(graph)};
}
