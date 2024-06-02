#include "grid.hpp"
#include <SFML/Graphics/RectangleShape.hpp>
#include "utils/math.hpp"

void GridDecomposition::Draw(sf::RenderTarget& rt) {
	sf::RectangleShape rect;
	rect.setSize(sf::Vector2f(CellSize));
	for (auto occupied : OccupiedIndices) {
		rect.setPosition(sf::Vector2f(Bounds.getPosition() + occupied.cwiseMul(CellSize)));
		rect.setFillColor(sf::Color(255, 255, 255, 80));
		rt.draw(rect);
	}
}

GridDecomposition GridDecomposition::Make(sf::Vector2i cell_size, sf::IntRect bounds, const std::vector<Wall>& walls) {
	GridDecomposition grid;
	grid.CellSize = cell_size;
	grid.Bounds = bounds;
	
	for (int x = 0; x < bounds.getSize().x / cell_size.x; x++) {
		for (int y = 0; y < bounds.getSize().x / cell_size.x; y++) {
			sf::FloatRect cell = (sf::FloatRect)grid.GetCellByIndex({x, y});

			for (const auto& wall : walls) {

				if(Math::LineRectIntersection(sf::Vector2f(wall.Start), sf::Vector2f(wall.End), cell)){
					grid.OccupiedIndices.push_back({x, y});
					break;
				}
			}

		}
	}

	return grid;
}

sf::IntRect GridDecomposition::GetCellByIndex(sf::Vector2i index)const{
	return {Bounds.getPosition() + sf::Vector2i(index.x, index.y).cwiseMul(CellSize), CellSize};
}

sf::Vector2i GridDecomposition::PositionToCellIndex(sf::Vector2i position)const{
	return LocalPositionToCellIndex(position - Bounds.getPosition());
}

sf::Vector2i GridDecomposition::LocalPositionToCellIndex(sf::Vector2i position) const{
	if(!Bounds.contains(position))
		return sf::Vector2i(-1, -1);

	return position.cwiseDiv(CellSize);
}

sf::Vector2i GridDecomposition::CellIndexToMiddlePosition(sf::Vector2i cell_index)const {
	return Bounds.getPosition() + cell_index.cwiseMul(CellSize) + CellSize / 2;
}

bool GridDecomposition::IsOccupied(sf::Vector2i cell_index)const {
	return std::find(OccupiedIndices.begin(), OccupiedIndices.end(), cell_index) != OccupiedIndices.end();
}

bool GridDecomposition::IsInBounds(sf::Vector2i index)const{
	return index.x < Size().x
		&& index.y < Size().y
		&& index.x >= 0
		&& index.y >= 0;
}

bool GridDecomposition::IsOccupied(sf::IntRect rect) const{
	for(int x = 0; x<rect.getSize().x; x++) {
		for (int y = 0; y < rect.getSize().y; y++) {
			sf::Vector2i grid_cell = rect.getPosition() + sf::Vector2i(x, y);
			
			if(IsOccupied(grid_cell))
				return true;
		}
	}
	return false;
}

sf::Vector2i GridDecomposition::Size()const {
	if(Bounds.getSize().x && Bounds.getSize().y)
		return Bounds.getSize().cwiseDiv(CellSize);
	return {0, 0};
}

bool GridDecomposition::IsOccupiedOrVisited(sf::Vector2i dst, const std::vector<sf::Vector2i>& visited)const{
	return IsOccupied(dst) || std::find(visited.begin(), visited.end(), dst) != visited.end();
}

bool GridDecomposition::HasObstacles(sf::Vector2i src, sf::Vector2i step, int count)const {
	for (int i = 0; i<count + 1; i++){
		if(IsOccupied(src + step * i))
			return true;
	}

	return false;
}
