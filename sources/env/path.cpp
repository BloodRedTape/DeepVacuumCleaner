#include "path.hpp"
#include <queue>
#include "bsl/assert.hpp"
#include "bsl/log.hpp"

std::vector<sf::Vector2i> BreadthSearchPathFinder::MakePath(const Environment& env)const {
	const auto &graph = env.CoverageGraph;

	std::vector<sf::Vector2i> path;
	
	auto start = env.LocalNearestToStartPosition();

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
	
	auto start = env.LocalNearestToStartPosition();

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
	
	auto start = env.LocalNearestToStartPosition();

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

std::vector<sf::Vector2i> DirectionSortPathBuilder::MakePath(const Environment& env) const{

	constexpr bool BackPropagate = false;
	
	const auto &graph = env.CoverageGraph;

	std::vector<sf::Vector2i> path;
	
	auto start = env.LocalStartPosition();
	path.push_back(start);

	auto start_nearest = env.LocalNearestToStartPosition();

	if(!start_nearest.has_value())	
		return {};

	path.push_back(start_nearest.value());
	
	auto GetFirstUnvisited = [&](const std::vector<sf::Vector2i> &candidates){
		for (auto next : candidates) {
			if (std::find(path.begin(), path.end(), next) == path.end()) {
				return std::make_optional(next);
			}
		}
		return std::optional<sf::Vector2i>();
	};

	auto TryGetPoint = [&](size_t last_index)->std::optional<sf::Vector2i>{
		if(last_index == 0)
			return std::nullopt;

		auto point = path[last_index];
		auto direction = point - path[last_index - 1];

		std::vector<sf::Vector2i> neighbours = graph[point].Neighbours;

		if (!verify(neighbours.size()))
			return std::nullopt;

		std::sort(neighbours.rbegin(), neighbours.rend(), SortByDirection{ direction });

		return GetFirstUnvisited(neighbours);
	};

	auto TryGetPointPropagate = [&]()->std::vector<sf::Vector2i>{
		int LastIndex = path.size() - 1;
		
		for (int i = LastIndex; i >= 1; --i) {
			auto point = TryGetPoint(i);

			if (point.has_value()) {
				std::vector<sf::Vector2i> chunk;
				
				if(BackPropagate){
					for(int back = LastIndex - 1; back >= i; --back)
						chunk.push_back(path[back]);
				}

				chunk.push_back(point.value());
				return chunk;
			}
		}
		return {};
	};
	
	for (;;) {
		auto next = TryGetPointPropagate();

		//if(!next.has_value())
		if(!next.size())
			break;
		
		std::copy(next.begin(), next.end(), std::back_inserter(path));
		//path.push_back(next.value());
	}


	return path;
}
