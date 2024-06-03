#include "graph.hpp"
#include "utils/render.hpp"

bool Graph::IsReachable(sf::Vector2i source, sf::Vector2i dst) const{
	std::vector<sf::Vector2i> path;
	
	path.push_back(source);
	
	for(int visited = 0; visited < path.size(); visited++){

		const Neighbours &neighbours = At(path[visited]);
		
		for(auto point: neighbours.Neighbours){
			if(point == dst)
				return true;

			path.push_back(point);
		}
	}

	return false;
}

void Graph::Draw(sf::RenderTarget& rt, sf::Vector2i offset)const {
	auto point_radius = 5.f;

	for (const auto& [vertex, neighbours] : m_Vertices) {
		for(const auto &neighbour: neighbours.Neighbours){
			auto src = vertex + offset;
			auto dst = neighbour + offset;
			auto dir = sf::Vector2f(dst - src).normalized();
			Render::DrawLine(rt, src, dst, 2.f, sf::Color::White);
			Render::DrawCircle(rt, dst - sf::Vector2i(dir * point_radius * 2.f), point_radius, sf::Color::Red);
		}
	}

	for (const auto& [vertex, neighbours] : m_Vertices) {
		Render::DrawCircle(rt, vertex + offset, point_radius, sf::Color::Green);
	}
}

Graph Graph::MakeFrom(const CoverageDecomposition& coverage_grid) {
	std::unordered_map<sf::Vector2i, Neighbours> graph;

	coverage_grid.ForEachCoverage([&](sf::Vector2i coverage) {
		auto points = coverage_grid.LocatedVisitPointsCache[coverage];
		auto neighbours = coverage_grid.GatherNeighboursVisitPoints(coverage);

		for (auto point : points) {
			graph[point].HasAnyOccupied = coverage_grid.HasAnyOccupied(coverage);
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
