#include "environment.hpp"
#include "math.hpp"
#include "bsl/serialization_std.hpp"
#include "render.hpp"
#include <fstream>
#include <SFML/Graphics.hpp>
#include <SFML/Window/Mouse.hpp>
#include "config.hpp"
#include "bsl/log.hpp"
#include <set>

DEFINE_LOG_CATEGORY(Env);

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
	if(!Bounds.contains(position))
		return sf::Vector2i(-1, -1);

	return (position - Bounds.getPosition()).cwiseDiv(CellSize);
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
	return Bounds.getSize().cwiseDiv(CellSize);
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

static int &At(sf::Vector2i& vec, int axis){
	if (axis == 0)
		return vec.x;
	if (axis == 1)
		return vec.y;
	assert(false);
	return vec.x;
};

static const int &At(const sf::Vector2i& vec, int axis){
	if (axis == 0)
		return vec.x;
	if (axis == 1)
		return vec.y;
	assert(false);
	return vec.x;
};


static bool IsRectInside(const sf::IntRect& inner, const sf::IntRect& outer) {
    return (
        inner.left >= outer.left &&
        inner.top >= outer.top &&
        inner.left + inner.width <= outer.left + outer.width &&
        inner.top + inner.height <= outer.top + outer.height
    );
}

struct PathBuilder {
	GridDecomposition &Grid;
	const std::size_t CoverageSize = 3;
	const sf::Vector2i CoverageGridSize;

	PathBuilder(GridDecomposition &grid, std::size_t coverage_size = 3):
		Grid(grid),
		CoverageSize(coverage_size),
		CoverageGridSize(
			std::ceil(grid.Bounds.getSize().x / (Grid.CellSize.x * float(coverage_size))),
			std::ceil(grid.Bounds.getSize().y / (Grid.CellSize.y * float(coverage_size)))
		)
	{}

	std::vector<sf::Vector2i> MakePath()const {
		return {};
	}

	sf::Vector2i GridToCoverageCell(sf::Vector2i grid_index)const{
		return grid_index / int(CoverageSize);
	}

	sf::IntRect CoverageCellRect(sf::Vector2i coverage_index)const{
		return {coverage_index * int(CoverageSize), sf::Vector2i(CoverageSize, CoverageSize)};
	}

	sf::IntRect Extend(sf::IntRect rect, int direction, int axis)const {
		assert(direction == 1 || direction == -1);
		assert(axis == 0 || axis == 1);


		sf::Vector2i min = rect.getPosition();
		sf::Vector2i max = rect.getPosition() + rect.getSize();

		At(min, axis) = std::min(At(min, axis), At(min, axis) + direction);
		At(max, axis) = std::max(At(max, axis), At(max, axis) + direction);

		return {min, max - min};
	}

	bool TryExtendInCoverageCell(sf::IntRect& rect, int direction, int axis)const{
		sf::IntRect new_rect = Extend(rect, direction, axis);
		
		if(Grid.IsOccupied(new_rect))
			return false;
		
		auto coverage_cell = CoverageCellRect(GridToCoverageCell(rect.getPosition()));

		//is out of bounds
		if(!IsRectInside(new_rect, coverage_cell))
			return false;
		
		rect = new_rect;
		return true;
	}

	sf::IntRect ExtendAsMuchInCoverageCell(sf::IntRect rect)const {
		while(TryExtendInCoverageCell(rect, 1, 0));
		while(TryExtendInCoverageCell(rect,-1, 0));
		while(TryExtendInCoverageCell(rect, 1, 1));
		while(TryExtendInCoverageCell(rect,-1, 1));

		return rect;
	}


	std::optional<sf::IntRect> MakeZoneFrom(sf::Vector2i point)const{
		if(Grid.IsOccupied(point))
			return std::nullopt;

		return ExtendAsMuchInCoverageCell({point, {1, 1}});
	}

	std::vector<sf::IntRect> MakeZoneDecomposition(sf::Vector2i coverage_cell)const {
		sf::IntRect coverage_cell_rect = CoverageCellRect(coverage_cell);
	
		std::vector<sf::IntRect> zones;

		for(int x = 0; x<coverage_cell_rect.getSize().x; x++) {
			for (int y = 0; y < coverage_cell_rect.getSize().y; y++) {
				sf::Vector2i grid_cell = coverage_cell_rect.getPosition() + sf::Vector2i(x, y);

				auto zone = MakeZoneFrom(grid_cell);

				if(zone.has_value() && std::find(zones.begin(), zones.end(), zone.value()) == zones.end())
					zones.push_back(zone.value());
			}
		}

		return zones;
	};

	bool TryExtendUntilFullCoverage(sf::IntRect& rect, int direction, int axis)const{
		sf::IntRect new_rect = Extend(rect, direction, axis);
		
		if(Grid.IsOccupied(new_rect))
			return false;
		
		if(At(new_rect.getSize(), axis) > CoverageSize)
			return false;
		
		rect = new_rect;
		return true;
	}


	std::optional<sf::IntRect> TryExtendUntilFullCoverage(sf::IntRect rect)const {
		while(TryExtendUntilFullCoverage(rect, 1, 0));
		while(TryExtendUntilFullCoverage(rect,-1, 0));
		while(TryExtendUntilFullCoverage(rect, 1, 1));
		while(TryExtendUntilFullCoverage(rect,-1, 1));

		if(rect.getSize() != sf::Vector2i(CoverageSize, CoverageSize))
			return std::nullopt;

		return rect;
	}

	std::vector<sf::IntRect> ToFullCoverageZones(const std::vector<sf::IntRect> &zones)const
	{
		std::vector<sf::IntRect> result;

		for (auto zone : zones) {
			auto full = TryExtendUntilFullCoverage(zone);

			if(full.has_value())
				result.push_back(full.value());
		}

		return result;
	}

	std::vector<sf::Vector2i> MakeVisitPoints(sf::Vector2i coverage_cell)const{
		auto full_coverage_zones = ToFullCoverageZones(MakeZoneDecomposition(coverage_cell));

		std::vector<sf::Vector2i> points;

		for (auto zone : full_coverage_zones) {
			sf::IntRect absoule{zone.getPosition().cwiseMul(Grid.CellSize), zone.getSize().cwiseMul(Grid.CellSize)};

			points.push_back(absoule.getCenter());
		}
		return points;
	}
};

std::vector<sf::Vector2i> GridDecomposition::BuildPath(sf::Vector2i start_position, int step)const {
	if (!Bounds.contains(start_position)) {
		LogEnv(Error, "Start is not in the bounds");
		return {};
	}

	sf::Vector2i start = PositionToCellIndex(start_position);

	if(start.x == -1 || start.y == -1){
		LogEnv(Error, "invalid start position cell");
		return {};
	}

	LogEnv(Error, "Building path for Grid: % | % = %", Size().x, Size().y, Size().x * Size().y);

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

bool Environment::IsFullfiled()const {
	return Path.size() > 0;
}

void Environment::DrawBounds(sf::RenderTarget& rt) {
	sf::FloatRect bounds = (sf::FloatRect)GatherBounds();

	if(bounds.getSize().length() < 1.f)
		return;

	const sf::Color BoundsColor = sf::Color::Magenta;

	Render::DrawLine(bounds.getPosition(), bounds.getPosition() + sf::Vector2f(0, bounds.getSize().y), 5.f, rt, BoundsColor);
	Render::DrawLine(bounds.getPosition(), bounds.getPosition() + sf::Vector2f(bounds.getSize().x, 0), 5.f, rt, BoundsColor);

	Render::DrawLine(bounds.getPosition() + bounds.getSize(), bounds.getPosition() + sf::Vector2f(0, bounds.getSize().y), 5.f, rt, BoundsColor);
	Render::DrawLine(bounds.getPosition() + bounds.getSize(), bounds.getPosition() + sf::Vector2f(bounds.getSize().x, 0), 5.f, rt, BoundsColor);
}

void Environment::Draw(sf::RenderTarget& rt, bool draw_numbers) {
	
	constexpr float PointRadius = 5.f;

	Render::DrawString(sf::Vector2f(StartPosition) - sf::Vector2f(0, PointRadius), "Start", rt);
	Render::DrawCircle(sf::Vector2f(StartPosition), PointRadius, rt, sf::Color::Cyan);

	sf::CircleShape shape(PointRadius);

	for (int i = 0; i<Path.size(); i++) {
		auto point = Path[i];
		shape.setPosition((sf::Vector2f)point);
		shape.setOrigin({PointRadius, PointRadius});
		rt.draw(shape);
		if(draw_numbers)
			Render::DrawString(sf::Vector2f(point) + sf::Vector2f(0, PointRadius), std::to_string(i), rt);
	}


	for (const auto &wall : Walls) {
		Render::DrawLine(sf::Vector2f(wall.Start), sf::Vector2f(wall.End), WallHeight, rt);
	}
}
void Environment::DrawZones(sf::RenderTarget& rt, sf::Vector2i mouse_position, bool zone, bool full_zone, bool points, bool cell_outline) {

	auto cell = Grid.PositionToCellIndex(mouse_position);

	if(cell.x == -1 || cell.y == -1)
		return;

	PathBuilder builder(Grid, 4);

	if(zone){
		auto zones = builder.MakeZoneDecomposition(builder.GridToCoverageCell(cell));

		for (auto zone : zones) {
			sf::RectangleShape rect;
			rect.setSize(Grid.CellRectToAbsolute(zone).getSize());
			rect.setPosition(Grid.CellRectToAbsolute(zone).getPosition());

			rect.setFillColor(sf::Color(255, 255, 255, 20));
			rect.setOutlineThickness(2);
			rect.setOutlineColor(sf::Color::Cyan);
			rt.draw(rect);
		}

	}
	if (full_zone) {
		auto zones = builder.MakeZoneDecomposition(builder.GridToCoverageCell(cell));

		for (auto zone : builder.ToFullCoverageZones(zones)) {
			sf::RectangleShape rect;
			rect.setSize(Grid.CellRectToAbsolute(zone).getSize());
			rect.setPosition(Grid.CellRectToAbsolute(zone).getPosition());

			rect.setFillColor(sf::Color(255, 255, 255, 20));
			rect.setOutlineThickness(2);
			rect.setOutlineColor(sf::Color::Green);
			rt.draw(rect);
		}
	}

	if (points) {
		auto points = builder.MakeVisitPoints(builder.GridToCoverageCell(cell));

		for (auto point: points)
			Render::DrawCircle(sf::Vector2f(Grid.Bounds.getPosition() + point), 5.f, rt, sf::Color::Green);
	}

	if (cell_outline) {
		auto coverage_rect = builder.CoverageCellRect(builder.GridToCoverageCell(cell));

		sf::RectangleShape rect;
		rect.setSize(Grid.CellRectToAbsolute(coverage_rect).getSize());
		rect.setPosition(Grid.CellRectToAbsolute(coverage_rect).getPosition());

		rect.setFillColor(sf::Color(255, 255, 255, 0));
		rect.setOutlineThickness(1);
		rect.setOutlineColor(sf::Color::Magenta);
		rt.draw(rect);
	}

	//Render::DrawString(sf::Vector2f(mouse_position) + sf::Vector2f(0, 30), Format("%\n% %", zones.size(), cell.x, cell.y), rt);
}

void Environment::SaveToFile(const std::string& filename) {
	std::fstream file(filename, std::ios::binary | std::ios::out);

	assert(file.is_open());
	
	Serializer<std::vector<sf::Vector2i>>::ToStream(Path, file);
	Serializer<std::vector<Wall>>::ToStream(Walls, file);
}

void Environment::LoadFromFile(const std::string& filename) {
	std::fstream file(filename, std::ios::binary | std::ios::in);

	assert(file.is_open());

	auto path = Serializer<std::vector<sf::Vector2i>>::FromStream(file);
	auto walls = Serializer<std::vector<Wall>>::FromStream(file);

	if(!path.has_value() || !walls.has_value())
		return;

	Path = std::move(path.value());
	Walls = std::move(walls.value());
}

void Environment::AutogeneratePath(sf::Vector2i cell_size, sf::Vector2i start_position, int step) {
	LogEnvIf(Path.size(), Warning, "Path is already generated, overwriting");
	Path.clear();
	StartPosition = start_position;

	Grid = GridDecomposition::Make(cell_size, GatherBounds(), Walls);

	Path = Grid.BuildPath(start_position, step);
}

sf::Vector2i Min(sf::Vector2i first, sf::Vector2i second) {
	return { std::min(first.x, second.x), std::min(first.y, second.y) };
}

sf::Vector2i Max(sf::Vector2i first, sf::Vector2i second) {
	return { std::max(first.x, second.x),std::max(first.y, second.y) };
}

sf::IntRect Environment::GatherBounds()const {
	if(!Walls.size())
		return {};

	sf::Vector2i min = Walls.front().Start;
	sf::Vector2i max = Walls.front().End;
	
	for (const auto &wall : Walls) {
		min = Min(min, wall.Start);
		min = Min(min, wall.End);

		max = Max(max, wall.Start);
		max = Max(max, wall.End);
	}

	return {min, max - min};
}
