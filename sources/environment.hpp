#pragma once

#include <optional>
#include <vector>
#include <SFML/System/Vector2.hpp>
#include <SFML/Graphics/RenderTarget.hpp>
#include <ostream>
#include <istream>

struct Wall {
	sf::Vector2i Start;
	sf::Vector2i End;

	static float TraceNearestObstacle(sf::Vector2f position, sf::Vector2f direction, const std::vector<Wall> &walls);
};

namespace Serialization {

	inline void ToStream(const Wall &wall, std::ostream& stream) {
		stream.write((char*)&wall, sizeof(wall));
	}

	inline Wall FromStreamImpl(std::istream& stream, Wall*) {
		Wall f;
		stream.read((char*)&f, sizeof(f));
		return f;
	}
}

struct Environment {
	std::vector<sf::Vector2i> Path;
	std::vector<Wall> Walls;


	bool IsFullfiled()const;

	void Draw(sf::RenderTarget& rt);

	void SaveToFile(const std::string& filename);

	void LoadFromFile(const std::string& filename);
};
