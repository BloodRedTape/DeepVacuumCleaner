#include "graph.hpp"
#include "utils/render.hpp"
#include "utils/math.hpp"
#include <queue>
#include <unordered_set>

void Graph::MakeConnection(sf::Vector2i src, sf::Vector2i dst, bool is_oriented){
	m_Vertices[src].AddUnique(dst);

	if(!is_oriented)
		m_Vertices[dst].AddUnique(src);
}

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


void Graph::Draw(sf::RenderTarget& rt, sf::Vector2i offset, bool draw_directions)const {

	for (const auto& [vertex, _] : m_Vertices) {
		DrawVertex(rt, vertex, offset, draw_directions);
	}
}

void Graph::DrawVertex(sf::RenderTarget& rt, sf::Vector2i vertex, sf::Vector2i offset, bool draw_directions) const	{
	auto RenderPointRadius = 5.f;
	const auto &neighbours = m_Vertices[vertex];
	for(const auto &neighbour: neighbours.Neighbours){
		auto src = vertex + offset;
		auto dst = neighbour + offset;
		auto dir = sf::Vector2f(dst - src).normalized();
		Render::DrawLine(rt, src, dst, 2.f, sf::Color::White);
		if(draw_directions)
			Render::DrawCircle(rt, dst - sf::Vector2i(dir * RenderPointRadius * 2.f), RenderPointRadius, sf::Color::Red);
	}

	Render::DrawCircle(rt, vertex + offset, RenderPointRadius, sf::Color::Green);
}

//we don't include Src point
std::vector<sf::Vector2i> Graph::ShortestPath(sf::Vector2i src, sf::Vector2i dst) const {
	auto Heuristic = [](const sf::Vector2i& a, const sf::Vector2i& b)->int {
		return std::abs(a.x - b.x) + std::abs(a.y - b.y);
	};

    std::unordered_map<sf::Vector2i, sf::Vector2i, std::hash<sf::Vector2i>> came_from;     // To reconstruct the path
    std::unordered_map<sf::Vector2i, int, std::hash<sf::Vector2i>> cost_so_far;            // Cost from src to n

    auto compare = [](const std::pair<sf::Vector2i, int>& a, const std::pair<sf::Vector2i, int>& b){
        return a.second > b.second;
    };
    std::priority_queue<std::pair<sf::Vector2i, int>, std::vector<std::pair<sf::Vector2i, int>>, decltype(compare)> frontier(compare);

    frontier.emplace(src, 0);
    came_from[src] = src;
    cost_so_far[src] = 0;

    while (!frontier.empty()) {
        sf::Vector2i current = frontier.top().first;
        frontier.pop();

        if (current == dst) {
            break;
        }

        const auto& neighbors = m_Vertices[current].Neighbours;
        for (const auto& next : neighbors) {
            int new_cost = cost_so_far[current] + 1; // Assuming the graph is unweighted (cost=1 for each move)
            if (cost_so_far.find(next) == cost_so_far.end() || new_cost < cost_so_far[next]) {
                cost_so_far[next] = new_cost;
                int priority = new_cost + Heuristic(next, dst);
                frontier.emplace(next, priority);
                came_from[next] = current;
            }
        }
    }

    std::vector<sf::Vector2i> path;
    if (came_from.find(dst) == came_from.end()) {
        return path; // No path found
    }

    for (sf::Vector2i current = dst; current != src; current = came_from[current]) {
        path.push_back(current);
    }
    path.push_back(src);
    std::reverse(path.begin(), path.end());
    return path;
}

std::size_t Graph::CountReachableFrom(sf::Vector2i src) const{
    std::queue<sf::Vector2i> frontier;
    std::unordered_set<sf::Vector2i, std::hash<sf::Vector2i>> visited;

    frontier.push(src);
    visited.insert(src);

    std::size_t count = 0;

    while (!frontier.empty()) {
        sf::Vector2i current = frontier.front();
        frontier.pop();
        count++;

        const auto& neighbors = m_Vertices.at(current).Neighbours;
        for (const auto& next : neighbors) {
            if (visited.find(next) == visited.end()) {
                frontier.push(next);
                visited.insert(next);
            }
        }
    }

    return count;
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

		if(!points.size())
			return;

		//std::unordered_map<sf::Vector2i, Neighbours> local_graph;

		for (auto point : points) {
			graph[point].HasAnyOccupied = coverage_grid.HasAnyOccupied(coverage);

			//XXX Do this only all points inside AreDirectlyReachable
			for (auto neighbour : points) {
				if (neighbour != point) {
					graph[point].AddUnique(neighbour);
					graph[neighbour].AddUnique(point);
				}
			}
		}

		//for (const auto &[v, n] : local_graph)
		//	graph[v] = n;

		auto start = coverage - sf::Vector2i(1, 1);
		auto end = coverage + sf::Vector2i(1, 1);

		for (int x = start.x; x <= end.x; x++) {
			for (int y = start.y; y <= end.y; y++) {
				if(x == coverage.x && y == coverage.y)
					continue;

				sf::Vector2i direction = sf::Vector2i(x, y) - coverage;

				std::sort(points.begin(), points.end(), SortByDirection{direction});

				auto edge_point = points.back();

				auto neighbours = coverage_grid.LocatedVisitPointsCache[{x, y}];

				if(!neighbours.size())
					continue;
				
				std::sort(neighbours.begin(), neighbours.end(), SortByDistanceTo{edge_point});

				//add nearest reachable one
				for (auto neighbour : neighbours) {
					if(coverage_grid.AreDirectlyReachable(edge_point, neighbour)){
						graph[edge_point].AddUnique(neighbour);
						graph[neighbour].AddUnique(edge_point);
						break;
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

bool SortByDirection::operator()(sf::Vector2i l, sf::Vector2i r) const{
	auto sort_dir = sf::Vector2f(Direction).normalized();

	return sf::Vector2f(l).dot(sort_dir) < sf::Vector2f(r).dot(sort_dir);
}

bool SortByDistanceTo::operator()(sf::Vector2i l, sf::Vector2i r) const{
	return sf::Vector2f(l - Point).length() < sf::Vector2f(r - Point).length();
}

bool SortByAngleCouterClockwize::operator()(sf::Vector2i l, sf::Vector2i r) const{
	auto forward = sf::Vector2f(Forward);
	return Math::AngleCouterClockwize(forward, sf::Vector2f(l - Center)) < Math::AngleCouterClockwize(forward, sf::Vector2f(r - Center));
}
