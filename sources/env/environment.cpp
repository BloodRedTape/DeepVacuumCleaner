#include "environment.hpp"
#include "utils/math.hpp"
#include "bsl/serialization_std.hpp"
#include "utils/render.hpp"
#include <fstream>
#include <SFML/Graphics.hpp>
#include <SFML/Window/Mouse.hpp>
#include "config.hpp"
#include "bsl/log.hpp"
#include <set>
#include <cmath>

DEFINE_LOG_CATEGORY(Env);

bool Environment::IsFullfiled()const {
	return Path.size() > 0;
}

void Environment::DrawBounds(sf::RenderTarget& rt) {
	sf::FloatRect bounds = (sf::FloatRect)GatherBounds();

	if(bounds.getSize().length() < 1.f)
		return;

	const sf::Color BoundsColor = sf::Color::Magenta;

	Render::DrawLine(rt, bounds.getPosition(), bounds.getPosition() + sf::Vector2f(0, bounds.getSize().y), 5.f, BoundsColor);
	Render::DrawLine(rt, bounds.getPosition(), bounds.getPosition() + sf::Vector2f(bounds.getSize().x, 0), 5.f, BoundsColor);

	Render::DrawLine(rt, bounds.getPosition() + bounds.getSize(), bounds.getPosition() + sf::Vector2f(0, bounds.getSize().y), 5.f, BoundsColor);
	Render::DrawLine(rt, bounds.getPosition() + bounds.getSize(), bounds.getPosition() + sf::Vector2f(bounds.getSize().x, 0), 5.f, BoundsColor);
}

void Environment::Draw(sf::RenderTarget& rt, std::size_t path_drawing_mode) {
	
	constexpr float PointRadius = 5.f;

	Render::DrawString(rt, StartPosition - sf::Vector2i(0, PointRadius), "Start");
	Render::DrawCircle(rt, StartPosition, PointRadius, sf::Color::Cyan);

	if(path_drawing_mode == PathWithPoints){
		for (int i = 0; i<Path.size(); i++) {
			auto point = Path[i];
			
			Render::DrawCircle(rt, point, PointRadius);
			Render::DrawString(rt, point + sf::Vector2i(0, PointRadius), std::to_string(i));
		}
	}
	if (path_drawing_mode == PathWithLines && Path.size()) {
		for (int i = 0; i<Path.size() - 1; i++) {
			auto start = Path[i];
			auto end = Path[i + 1];
			Render::DrawLine(rt, start, end, 3.f);
		}
	}


	for (const auto &wall : Walls)
		Render::DrawLine(rt, wall.Start, wall.End, RenderWallHeight);
}

static void DrawForCell(const CoverageDecomposition& builder, sf::RenderTarget& rt, sf::Vector2i coverage_cell, bool zone, bool full_zone, bool points, bool cell_outline) {
	if (zone) {
		auto zones = builder.MakeZoneDecomposition(coverage_cell);

		for (auto zone : zones) {
			Render::DrawRect(rt, builder.Grid.CellRectToAbsolute(zone), sf::Color::Cyan * sf::Color(255, 255, 255, 40), 2, sf::Color::Cyan);
		}

	}
	if (full_zone) {
		auto zones = builder.MakeZoneDecomposition(coverage_cell);

		for (auto zone : builder.ToFullCoverageZones(zones)) {
			Render::DrawRect(rt, builder.Grid.CellRectToAbsolute(zone), sf::Color::Green * sf::Color(255, 255, 255, 20), 2, sf::Color::Green);
		}
	}

	if (points) {
		auto points = builder.GatherCoverageVisitPoints(coverage_cell);

		for (auto point : points)
			Render::DrawCircle(rt, builder.Grid.Bounds.getPosition() + point, 5.f, sf::Color::Green);
	}

	if (cell_outline) {
		auto coverage_rect = builder.CoverageCellRect(coverage_cell);

		Render::DrawRect(rt, builder.Grid.CellRectToAbsolute(coverage_rect), sf::Color(255, 255, 255, 0), 1, sf::Color::Magenta);
	}
}

void Environment::DrawZones(sf::RenderTarget& rt, sf::Vector2i world_mouse_position, bool for_all_cells, bool zone, bool full_zone, bool points, bool cell_outline) {

	bool debug_line_trace = false;

	if(debug_line_trace){
		for (auto cell : Coverage.TraceLine(StartPosition - Grid.Bounds.getPosition(), world_mouse_position - Grid.Bounds.getPosition())) {
			Render::DrawRect(rt, Grid.Bounds.getPosition() + cell.cwiseMul(Grid.CellSize), Grid.CellSize);
		}
	}
#if 0
	if (true) {
		for(int x = 0; x < builder.CoverageGridSize.x; x++){
			for(int y = 0; y < builder.CoverageGridSize.y; y++){
				sf::Vector2i coverage_cell(x, y);

				auto visit_points = builder.MakeVisitPoints(coverage_cell);
				if(visit_points.size() > 1 || builder.HasAnyOccupied(coverage_cell))
				{
					for (auto point : visit_points)
						Render::DrawCircle(rt, builder.Grid.Bounds.getPosition() + point, 5.f, sf::Color::Green);
				}
			}
		}
	}
#else
	if(!for_all_cells){
		auto cell = Grid.PositionToCellIndex(world_mouse_position);

		if(cell.x == -1 || cell.y == -1)
			return;


		auto coverage_cell = Coverage.GridToCoverageCell(cell);

		DrawForCell(Coverage, rt, coverage_cell, zone, full_zone, points, cell_outline);
	} else {
		for(int x = 0; x < Coverage.CoverageGridSize.x; x++){
			for(int y = 0; y < Coverage.CoverageGridSize.y; y++){
				sf::Vector2i coverage_cell(x, y);
				DrawForCell(Coverage, rt, coverage_cell, zone, full_zone, points, cell_outline);
			}
		}
	}
#endif
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
	Coverage.Rebuild();

	//Path = Grid.BuildPath(start_position, step);

	Path = Coverage.BuildPath(start_position);
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
