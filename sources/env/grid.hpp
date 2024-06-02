#pragma once

#include <vector>
#include <SFML/System/Vector2.hpp>
#include <SFML/Graphics/Rect.hpp>
#include <SFML/Graphics/RenderTarget.hpp>
#include "env/wall.hpp"

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

