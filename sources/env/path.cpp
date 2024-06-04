#include "path.hpp"
#include <queue>
#include "bsl/assert.hpp"

std::vector<sf::Vector2i> BreadthSearchPathFinder::MakePath(const Environment& env)const {
	const auto &graph = env.CoverageGraph;

	std::vector<sf::Vector2i> path;
	
	auto start = env.Coverage.LocalNearestVisitPointTo(env.StartPosition - env.Grid.Bounds.getPosition());

	if(!verify(start.has_value()))
		return {};

	path.push_back(start.value());
	
	for(int visited = 0; visited < path.size(); visited++){

		const Neighbours &neighbours = graph[path[visited]];
		
		for(auto point: neighbours.Neighbours){
			if(std::find(path.begin(), path.end(), point) == path.end())
				path.push_back(point);
		}
	}

	return path;
}

std::vector<sf::Vector2i> BreadthSearchWithSortPathFinder::MakePath(const Environment& env)const {
	const auto &graph = env.CoverageGraph;

	std::vector<sf::Vector2i> path;
	
	auto start = env.Coverage.LocalNearestVisitPointTo(env.StartPosition - env.Grid.Bounds.getPosition());

	if(!verify(start.has_value()))
		return {};

	path.push_back(start.value());
	
	for(int visited = 0; visited < path.size(); visited++){
		auto source = path[visited];
		auto neighbours = graph[source].Neighbours;

		std::sort(neighbours.begin(), neighbours.end(), [source](sf::Vector2i l, sf::Vector2i r) {
			return sf::Vector2f(l - source).length() < sf::Vector2f(r - source).length();
		});
		
		for(auto point: neighbours){
			if(std::find(path.begin(), path.end(), point) == path.end())
				path.push_back(point);
		}
	}

	return path;
}

std::vector<sf::Vector2i> FirstNearWallPathBuilder::MakePath(const Environment& env) const
{
	const auto &graph = env.CoverageGraph;

	std::vector<sf::Vector2i> path;
	
	auto start = env.Coverage.LocalNearestVisitPointTo(env.StartPosition - env.Grid.Bounds.getPosition());

	if(!verify(start.has_value()))
		return {};

	path.push_back(start.value());
	
	for(int visited = 0; visited < path.size(); visited++){
		auto source = path[visited];
		auto neighbours = graph[source].Neighbours;

		std::sort(neighbours.begin(), neighbours.end(), [](sf::Vector2i l, sf::Vector2i r) {
			return sf::Vector2f(l).length() < sf::Vector2f(r).length();
		});
		
		for(auto point: neighbours){
			if(std::find(path.begin(), path.end(), point) == path.end())
				path.push_back(point);
		}
	}

	return path;
}
