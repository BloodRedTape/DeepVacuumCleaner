#include "wall.hpp"
#include <optional>
#include "utils/math.hpp"

float Wall::TraceNearestObstacle(sf::Vector2f position, sf::Vector2f direction, const std::vector<Wall> &walls){
	std::optional<float> nearest;
	
	for (const auto &wall : walls) {
		auto result = Math::RayLineIntersection(position, direction, sf::Vector2f(wall.Start), sf::Vector2f(wall.End));

		if(!nearest.has_value()){
			nearest = result;
			continue;
		}

		if (result.has_value() && result.value() < nearest.value()) {
			nearest = result;
		}
	}
	
	return nearest.has_value() ? nearest.value() : 9999999999;
}

std::pair<float, sf::Vector2f> Wall::TraceNearestObstacleWithNormal(sf::Vector2f position, sf::Vector2f direction, const std::vector<Wall> &walls){
	std::optional<std::pair<float, sf::Vector2f>> nearest;
	
	for (const auto &wall : walls) {
		auto result = Math::RayLineIntersectionWithNormal(position, direction, sf::Vector2f(wall.Start), sf::Vector2f(wall.End));

		if(!nearest.has_value()){
			nearest = result;
			continue;
		}

		if (result.has_value() && result.value().first < nearest.value().first) {
			nearest = result;
		}
	}
	
	return nearest.has_value() ? nearest.value() : std::make_pair( 9999999999.f, sf::Vector2f{} );
}
