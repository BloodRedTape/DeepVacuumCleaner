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

	std::vector<sf::Vector2i> BuildPath(sf::Vector2i start_position, int step)const;

	sf::IntRect GetCellByIndex(sf::Vector2i index)const;

	sf::Vector2i PositionToCellIndex(sf::Vector2i position)const;

	sf::Vector2i LocalPositionToCellIndex(sf::Vector2i position)const;

	sf::Vector2i CellIndexToMiddlePosition(sf::Vector2i cell_index)const;

	bool IsOccupied(sf::Vector2i cell_index)const;

	bool IsInBounds(sf::Vector2i cell_index)const;

	bool IsOccupied(sf::IntRect rect)const;

	bool IsOccupiedOrVisited(sf::Vector2i dst, const std::vector<sf::Vector2i>& visited)const;

	bool HasObstacles(sf::Vector2i src, sf::Vector2i step, int count)const;

	sf::Vector2i GridPosition()const{ return Bounds.getPosition(); }

	sf::FloatRect CellRectToAbsolute(sf::IntRect rect)const{ return {sf::Vector2f(GridPosition() + rect.getPosition().cwiseMul(CellSize)), sf::Vector2f(rect.getSize().cwiseMul(CellSize))}; }

	sf::Vector2i Size()const;

	static GridDecomposition Make(sf::Vector2i cell_size, sf::IntRect bounds, const std::vector<Wall> &walls);
};

struct Environment {
	std::vector<sf::Vector2i> Path;
	std::vector<Wall> Walls;
	sf::Vector2i StartPosition;
	
	GridDecomposition Grid;
	std::size_t CoverageSize = 4;

	int RenderWallHeight = 4.f;
	static constexpr int NoPath = 0;
	static constexpr int PathWithPoints = 1;
	static constexpr int PathWithLines = 2;

	bool IsFullfiled()const;

	void DrawBounds(sf::RenderTarget& rt);

	void Draw(sf::RenderTarget& rt, std::size_t path_drawing_mode = 0);

	void DrawZones(sf::RenderTarget &rt, sf::Vector2i mouse_position, bool for_all_cells, bool zone, bool full_zone, bool points, bool cell);

	void SaveToFile(const std::string& filename);

	void LoadFromFile(const std::string& filename);

	void AutogeneratePath(sf::Vector2i cell_size, sf::Vector2i start_position, int step);

	sf::IntRect GatherBounds()const;

	void Clear() {
		*this = Environment();
	}
};
