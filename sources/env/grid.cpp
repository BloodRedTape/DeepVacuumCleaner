#include "grid.hpp"
#include <SFML/Graphics/RectangleShape.hpp>
#include "utils/math.hpp"
#include "bsl/log.hpp"

DEFINE_LOG_CATEGORY(Grid)

void GridDecomposition::Draw(sf::RenderTarget& rt) {
	sf::RectangleShape rect;
	rect.setSize(sf::Vector2f(CellSize, CellSize));
	for (auto occupied : OccupiedIndices) {
		rect.setPosition(sf::Vector2f(Bounds.getPosition() + occupied.cwiseMul(CellSizeVec())));
		rect.setFillColor(sf::Color(255, 255, 255, 80));
		rt.draw(rect);
	}
}


std::vector<sf::Vector2i> GridDecomposition::BuildPath(sf::Vector2i start_position, int step)const {
	if (!Bounds.contains(start_position)) {
		LogGrid(Error, "Start is not in the bounds");
		return {};
	}

	sf::Vector2i start = PositionToCellIndex(start_position);

	if(start.x == -1 || start.y == -1){
		LogGrid(Error, "invalid start position cell");
		return {};
	}

	LogGrid(Error, "Building path for Grid: % | % = %", Size().x, Size().y, Size().x * Size().y);

	std::vector<sf::Vector2i> index_path;
	
	index_path.push_back(start);

	sf::Vector2i directions[] = {
		{ 0, 1},
		{ 1, 0},
		{ 0,-1},
		{-1, 0},
	};

	auto TryGetNextCell = [&](const std::vector<sf::Vector2i>& visited) -> std::optional<sf::Vector2i> {
		const int Step = step;

		for (auto it = visited.rbegin(); it != visited.rend(); ++it) {
			sf::Vector2i current = *it;

			for (auto dir : directions) {
				auto candidate = current + dir * Step;

				if (IsInBounds(candidate) && !IsOccupiedOrVisited(candidate, visited) && !HasObstacles(current, dir, Step))
					return { candidate };
			}
		}

		return std::nullopt;
	};

	for(;;){
		auto next = TryGetNextCell(index_path);

		if(!next.has_value())
			break;
		
		index_path.push_back(next.value());
	}


	std::vector<sf::Vector2i> path;

	for(auto index: index_path)
		path.push_back(CellIndexToMiddlePosition(index));

	return path;	
}


GridDecomposition GridDecomposition::Make(std::size_t cell_size, sf::IntRect bounds, const std::vector<Wall>& walls) {
	GridDecomposition grid;
	grid.CellSize = cell_size;
	grid.Bounds = bounds;
	
	for (int x = 0; x < bounds.getSize().x / cell_size; x++) {
		for (int y = 0; y < bounds.getSize().y / cell_size; y++) {
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
	return {Bounds.getPosition() + sf::Vector2i(index.x, index.y).cwiseMul(CellSizeVec()), CellSizeVec() };
}

sf::Vector2i GridDecomposition::PositionToCellIndex(sf::Vector2i position)const{
	return LocalPositionToCellIndex(position - Bounds.getPosition());
}

sf::Vector2i GridDecomposition::LocalPositionToCellIndex(sf::Vector2i position) const{
	if(!verify(position.x >= 0 && position.y >= 0))
		return sf::Vector2i(-1, -1);

	return position.cwiseDiv(CellSizeVec());
}

sf::Vector2i GridDecomposition::CellIndexToMiddlePosition(sf::Vector2i cell_index)const {
	return Bounds.getPosition() + cell_index.cwiseMul(CellSizeVec()) + CellSizeVec() / 2;
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
		return Bounds.getSize().cwiseDiv(CellSizeVec());
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
