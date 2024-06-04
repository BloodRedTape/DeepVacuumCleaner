#include "path.hpp"
#include <queue>
#include "bsl/assert.hpp"
#include "bsl/log.hpp"

template<typename TryGetNextPointType>
inline std::vector<sf::Vector2i> PathBuilder::TryGetPointWithBackPropagation(const Environment &env, const std::vector<sf::Vector2i>& path, TryGetNextPointType TryGetNextPoint, bool include_back_path, bool optimize_back_path) const
{
	int LastIndex = path.size() - 1;
		
	for (int i = LastIndex; i >= 1; --i) {
		auto point = TryGetNextPoint(i);

		if (point.has_value()) {
			std::vector<sf::Vector2i> chunk;
			
			if(include_back_path){
				const int BackPathStart = LastIndex - 1;

				if (!optimize_back_path) {
					const int BackPathEnd = i;
					for(int back = BackPathStart; back >= BackPathEnd; --back)
						chunk.push_back(path[back]);
					chunk.push_back(point.value());
				} else {
					if(i != LastIndex) {
						chunk = env.CoverageGraph.ShortestPath(path[BackPathStart], point.value());
						verify(chunk.size());
					} else {
						chunk.push_back(point.value());
					}
				}
			}

			return chunk;
		}
	}
	return {};
}

std::optional<sf::Vector2i> PathBuilder::FindFirstUnvisited(const Environment& env, const std::vector<sf::Vector2i>& candidates, const std::vector<sf::Vector2i>& path, const std::optional<sf::Vector2i> except)const {
	for (auto next : candidates) {
		if (std::find(path.begin(), path.end(), next) == path.end()) {
			if(except.has_value() && except.value() == next)
				continue;

			return std::make_optional(next);
		}
	}
	return std::optional<sf::Vector2i>();
}

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

	const auto &graph = env.CoverageGraph;

	std::vector<sf::Vector2i> path;
	
	auto start = env.LocalStartPosition();
	path.push_back(start);

	auto start_nearest = env.LocalNearestToStartPosition();

	if(!start_nearest.has_value())	
		return {};

	path.push_back(start_nearest.value());

	auto TryGetPoint = [&](size_t last_index)->std::optional<sf::Vector2i>{
		if(last_index == 0)
			return std::nullopt;

		auto point = path[last_index];
		auto direction = point - path[last_index - 1];

		std::vector<sf::Vector2i> neighbours = graph[point].Neighbours;

		if (!verify(neighbours.size()))
			return std::nullopt;

		std::sort(neighbours.rbegin(), neighbours.rend(), SortByDirection{ direction });

		return FindFirstUnvisited(env, neighbours, path);
	};
	
	for (;;) {
		auto next = TryGetPointWithBackPropagation(env, path, TryGetPoint, false);

		//if(!next.has_value())
		if(!next.size())
			break;
		
		std::copy(next.begin(), next.end(), std::back_inserter(path));
		//path.push_back(next.value());
	}


	return path;
}

std::vector<sf::Vector2i> RightFirstPathBuilder::MakePath(const Environment& env) const{
	const auto &graph = env.CoverageGraph;

	std::vector<sf::Vector2i> path;

	auto start = env.LocalStartPosition();
	auto start_nearest = env.LocalNearestToStartPosition();

	if(!start_nearest.has_value())	
		return {};

	path.push_back(start);
	path.push_back(start_nearest.value());


	auto FindFirstUnvisitedByAngle = [&](sf::Vector2i point, sf::Vector2i prev, std::optional<sf::Vector2i> except = {})->std::optional<sf::Vector2i> {
		auto direction = point - prev;

		sf::Vector2i right(sf::Vector2f(direction).rotatedBy(sf::degrees(-90)));

		std::vector<sf::Vector2i> neighbours = graph[point].Neighbours;

		if (!verify(neighbours.size()))
			return std::nullopt;

		std::sort(neighbours.begin(), neighbours.end(), SortByAngleCouterClockwize{ right, point });

		return FindFirstUnvisited(env, neighbours, path, except);
	};

	auto FindFirstUnvisitedByDistance = [&](sf::Vector2i point, sf::Vector2i prev, std::optional<sf::Vector2i> except = {})->std::optional<sf::Vector2i> {
		auto direction = point - prev;

		std::vector<sf::Vector2i> neighbours = graph[point].Neighbours;

		if (!verify(neighbours.size()))
			return std::nullopt;

		std::sort(neighbours.begin(), neighbours.end(), SortByDistanceTo{ point });

		return FindFirstUnvisited(env, neighbours, path, except);
	};

	auto TryGetPoint = [&](std::size_t end)->std::optional<sf::Vector2i>{
		if(end == 0)
			return std::nullopt;

		auto point = path[end];
		auto prev = path[end - 1];

		auto by_angle = FindFirstUnvisitedByAngle(point, prev);
#ifdef MAKE_SMALL_OPTIMIZE_BY_DISTANCE
		auto by_distance = FindFirstUnvisitedByDistance(point, prev);

		if (by_distance.has_value()) {
			auto next_by_angle = FindFirstUnvisitedByAngle(by_distance.value(), point, point);
			auto next_by_distance = FindFirstUnvisitedByDistance(by_distance.value(), point, point);

			if(next_by_angle == next_by_distance && next_by_angle == by_angle)
				return by_distance;
		}
#endif
		return by_angle;
	};

	for (;;) {
		auto next = TryGetPointWithBackPropagation(env, path, TryGetPoint, true, true);

		if(!next.size())
			break;
		
		std::copy(next.begin(), next.end(), std::back_inserter(path));
	}

	return path;
}

