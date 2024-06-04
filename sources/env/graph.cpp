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
			//Render::DrawCircle(rt, dst - sf::Vector2i(dir * point_radius * 2.f), point_radius, sf::Color::Red);
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

Graph Graph::MakeOptimizedFrom(const CoverageDecomposition& coverage_grid){
	std::unordered_map<sf::Vector2i, Neighbours> graph;

	coverage_grid.ForEachCoverage([&](sf::Vector2i coverage) {
		auto points = coverage_grid.LocatedVisitPointsCache[coverage];
		//auto neighbours = coverage_grid.GatherNeighboursVisitPoints(coverage);

		for (auto point : points) {
			graph[point].HasAnyOccupied = coverage_grid.HasAnyOccupied(coverage);
			
			//XXX Do this only all points inside AreDirectlyReachable
			graph[point].AppendUnique(points);
			graph[point].Remove(point);
			
			if(points.size() != 1)
				continue;

			auto SortByDistanceToPoint = [point](sf::Vector2i l, sf::Vector2i r) {
				return sf::Vector2f(l - point).length() < sf::Vector2f(r - point).length();
			};

			auto start = coverage - sf::Vector2i(1, 1);
			auto end = coverage + sf::Vector2i(1, 1);

			for (int x = start.x; x <= end.x; x++) {
				for (int y = start.y; y <= end.y; y++) {
					if(x == coverage.x && y == coverage.y)
						continue;

					auto neighbours = coverage_grid.LocatedVisitPointsCache[{x, y}];

					if(!neighbours.size())
						continue;
					
					std::sort(neighbours.begin(), neighbours.end(), SortByDistanceToPoint);
					//add nearest reachable one
					for (auto neighbour : neighbours) {
						if(coverage_grid.AreDirectlyReachable(point, neighbour)){
							graph[point].AddUnique(neighbours.front());
							break;
						}
					}

				}				
			}
		}
	});


	return {std::move(graph)};
}

void Neighbours::AddUnique(sf::Vector2i neighbour){
	if(std::find(Neighbours.begin(), Neighbours.end(), neighbour) == Neighbours.end())
		Neighbours.push_back(neighbour);
}

void Neighbours::AppendUnique(const sf::Vector2i* points, std::size_t count){
	for (int i = 0; i < count; i++) {
		AddUnique(points[i]);
	}
}

void Neighbours::AppendUnique(const std::vector<sf::Vector2i>& points){
	AppendUnique(points.data(), points.size());
}

void Neighbours::Remove(sf::Vector2i point){
	auto it = std::remove(Neighbours.begin(), Neighbours.end(), point);

	if(it != Neighbours.end())
		Neighbours.pop_back();
}
