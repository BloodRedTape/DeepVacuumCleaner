#pragma once

#include <SFML/System/Vector2.hpp>
#include <vector>

struct Wall {
	sf::Vector2i Start;
	sf::Vector2i End;

	static float TraceNearestObstacle(sf::Vector2f position, sf::Vector2f direction, const std::vector<Wall> &walls);
};
