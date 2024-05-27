#include "environment.hpp"
#include "math.hpp"
#include "bsl/serialization_std.hpp"
#include "render.hpp"
#include <fstream>
#include <SFML/Graphics.hpp>
#include "config.hpp"
#include "bsl/log.hpp"

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
		return (assert(false), sf::Vector2i(-1, -1));

	return (position - Bounds.getPosition()).cwiseDiv(CellSize);
}

sf::Vector2i GridDecomposition::CellIndexToMiddlePosition(sf::Vector2i cell_index)const {
	return Bounds.getPosition() + cell_index.cwiseMul(CellSize) + CellSize / 2;
}

bool GridDecomposition::IsOccupied(sf::Vector2i cell_index)const {
	return std::find(OccupiedIndices.begin(), OccupiedIndices.end(), cell_index) != OccupiedIndices.end();
}

bool GridDecomposition::IsInBounds(sf::Vector2i index)const{
	return index.x < CellsCount().x
		&& index.y < CellsCount().y
		&& index.x >= 0
		&& index.y >= 0;
};

sf::Vector2i GridDecomposition::CellsCount()const {
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

	LogEnv(Error, "Building path for Grid: % | % = %", CellsCount().x, CellsCount().y, CellsCount().x * CellsCount().y);

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
