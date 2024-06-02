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
