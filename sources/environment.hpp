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


struct GridDecomposition {
	std::vector<sf::Vector2i> OccupiedIndices;
	sf::IntRect Bounds;
	sf::Vector2i CellSize;

	void Draw(sf::RenderTarget& rt);

	std::vector<sf::Vector2i> BuildPath(sf::Vector2i start_position)const;

	sf::IntRect GetCellByIndex(sf::Vector2i index)const;

	sf::Vector2i PositionToCellIndex(sf::Vector2i position)const;

	sf::Vector2i CellIndexToMiddlePosition(sf::Vector2i cell_index)const;

	bool IsOccupied(sf::Vector2i cell_index)const;

	sf::Vector2i CellsCount()const;

	static GridDecomposition Make(sf::Vector2i cell_size, sf::IntRect bounds, const std::vector<Wall> &walls);
};

struct Environment {
	std::vector<sf::Vector2i> Path;
	std::vector<Wall> Walls;
	sf::Vector2i StartPosition;
	
	GridDecomposition Grid;

	static constexpr float WallHeight = 4.f;

	bool IsFullfiled()const;

	void DrawBounds(sf::RenderTarget& rt);

	void Draw(sf::RenderTarget& rt, bool draw_numbers = true);

	void SaveToFile(const std::string& filename);

	void LoadFromFile(const std::string& filename);

	void AutogeneratePath(sf::Vector2i cell_size, sf::Vector2i start_position);

	sf::IntRect GatherBounds()const;
};
