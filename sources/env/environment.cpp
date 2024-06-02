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

struct AxisAlignedDirection2D{
	int Direction = 0;
	int Axis = 0;

	AxisAlignedDirection2D(int axis, int direction){
		Direction = direction;
		Axis = axis;

		assert(direction == 1 || direction == -1);
		assert(axis == 0 || direction == 2);
	}

	static std::optional<AxisAlignedDirection2D> Make(sf::Vector2i vector) {
		if (vector.x == 0 && std::abs(vector.y) == 1) {
			return AxisAlignedDirection2D(1, vector.y);
		}
		if (vector.y == 0 && std::abs(vector.x) == 1) {
			return AxisAlignedDirection2D(0, vector.x);
		}
		return std::nullopt;
	}
};

struct PathBuilder {
	GridDecomposition &Grid;
	const std::size_t CoverageSize = 3;
	const sf::Vector2i CoverageGridSize;
	std::vector<sf::Vector2i> VisitPoints;

	PathBuilder(GridDecomposition &grid, std::size_t coverage_size = 3):
		Grid(grid),
		CoverageSize(coverage_size),
		CoverageGridSize(
			std::ceil(grid.Bounds.getSize().x / (Grid.CellSize.x * float(coverage_size))),
			std::ceil(grid.Bounds.getSize().y / (Grid.CellSize.y * float(coverage_size)))
		)
	{
		VisitPoints = MakeVisitPoints();
	}

	std::vector<sf::Vector2i> MakePath()const {
		return {};
	}

	sf::Vector2i GridToCoverageCell(sf::Vector2i grid_index)const{
		return grid_index / int(CoverageSize);
	}

	sf::IntRect CoverageCellRect(sf::Vector2i coverage_index)const{
		return {coverage_index * int(CoverageSize), sf::Vector2i(CoverageSize, CoverageSize)};
	}

	sf::IntRect CellToLocalRect(sf::IntRect rect)const{
		return {rect.getPosition().cwiseMul(Grid.CellSize), rect.getSize().cwiseMul(Grid.CellSize)};
	}

	sf::IntRect CoverageCellLocalRect(sf::Vector2i coverage_index)const {
		return CellToLocalRect(CoverageCellRect(coverage_index));
	}

	bool IsInBounds(sf::Vector2i coverage_index)const {
		return coverage_index.x >= 0
			&& coverage_index.x < CoverageGridSize.x
			&& coverage_index.y >= 0
			&& coverage_index.y < CoverageGridSize.y;
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
	
	//Visit points are in world_local
	std::vector<sf::Vector2i> MakeVisitPoints(sf::Vector2i coverage_cell)const{
		auto full_coverage_zones = ToFullCoverageZones(MakeZoneDecomposition(coverage_cell));

		std::vector<sf::Vector2i> points;

		for (auto zone : full_coverage_zones) {
			sf::IntRect absoule{zone.getPosition().cwiseMul(Grid.CellSize), zone.getSize().cwiseMul(Grid.CellSize)};

			points.push_back(absoule.getCenter());
		}
		return points;
	}

	std::vector<sf::Vector2i> MakeVisitPoints()const {
		std::vector<sf::Vector2i> points;
		for (int x = 0; x < CoverageGridSize.x; x++) {
			for (int y = 0; y < CoverageGridSize.y; y++) {
				auto cell_points = MakeVisitPoints({x, y});
			
				// где сука points.append(cell_points)
				std::copy(cell_points.begin(), cell_points.end(), std::back_inserter(points));
			}
		}
		return points;
	}

	std::vector<sf::Vector2i> GatherCoverageVisitPoints(sf::Vector2i coverage_cell)const {
		auto rect = CoverageCellRect(coverage_cell);
		
		std::vector<sf::Vector2i> result;

		for (auto point : VisitPoints) {
			auto cell = Grid.LocalPositionToCellIndex(point);

			if(cell.x == -1)
				Println("Error");

			if(rect.contains(cell))
				result.push_back(point);
		}

		return result;
	}

	bool HasAnyOccupied(sf::Vector2i coverage)const {
		auto rect = CoverageCellRect(coverage);

		for (auto cell : Grid.OccupiedIndices) {
			if(rect.contains(cell))
				return true;
		}
		return false;
	}

	std::vector<sf::Vector2i> GatherCoverageVisitPointsInRadius(sf::Vector2i coverage_cell, sf::Vector2i dims = {1, 1})const {
		std::vector<sf::Vector2i> result;
		
		dims -= sf::Vector2i(1, 1);
		auto start = coverage_cell - dims;
		auto end = coverage_cell + dims;

		for (int x = start.x; x <= end.x; x++) {
			for (int y = start.y; y <= end.y; y++) {
				auto points = GatherCoverageVisitPoints({x, y});

				std::copy(points.begin(), points.end(), std::back_inserter(result));
			}				
		}
		
		return result;
	}

	std::vector<sf::Vector2i> TraceLine(sf::Vector2i local_src, sf::Vector2i local_dst)const {
		std::vector<sf::Vector2i> result;

		for (int x = 0; x < Grid.Size().x; x++) {
			for (int y = 0; y < Grid.Size().y; y++) {
				sf::FloatRect cell = (sf::FloatRect)Grid.GetCellByIndex({x, y});

				if(Math::LineRectIntersection(sf::Vector2f(Grid.Bounds.getPosition() + local_src), sf::Vector2f(Grid.Bounds.getPosition() + local_dst), cell)){
					result.push_back({x, y});
				}
			}
		}

		return result;
	}
	
	bool AreDirectlyReachable(sf::Vector2i local_src, sf::Vector2i local_dst)const {
		for (auto line_cell : TraceLine(local_src, local_dst)) {
			
			auto coverage = TryExtendUntilFullCoverage({line_cell, {1, 1}});

			if(!coverage.has_value())
				return false;
		}
		
		return true;
	}



	std::vector<sf::Vector2i> GetSortedReachableVisitPointsNotVisited(sf::Vector2i src_coverage, sf::Vector2i dst_coverage, sf::Vector2i src_local, sf::Vector2i dir_local, const std::vector<sf::Vector2i> &visited)const{
		sf::Vector2i direction = src_coverage - dst_coverage;
		auto axis_aligned_direction = AxisAlignedDirection2D::Make(direction);

		if(!axis_aligned_direction){
			Println("Can't make direction from (%, %)", direction.x, direction.y);
			return {};
		}

		std::vector<sf::Vector2i> reachable_visit_points;

		for (auto point : GatherCoverageVisitPointsInRadius(dst_coverage, {2, 2})) {
			
			if(!AreDirectlyReachable(src_local, point))
				continue;

			if(std::find(visited.begin(), visited.end(), point) != visited.end())
				continue;

			reachable_visit_points.push_back(point);
		}

		auto CompareByDirectionThenLength = [src_local, dir = axis_aligned_direction.value()](sf::Vector2i left, sf::Vector2i right) {
			auto l = At(left, dir.Axis) * dir.Direction;
			auto r = At(right, dir.Axis) * dir.Direction;
			
			if(l == r)
				return sf::Vector2f(left - src_local).length() < sf::Vector2f(right - src_local).length();

			return l > r;
		};

		auto CompareByDot = [src_local, dir_local = sf::Vector2f(dir_local).normalized()](sf::Vector2i left, sf::Vector2i right) {
			auto left_dir = sf::Vector2f(left - src_local).normalized();
			auto right_dir = sf::Vector2f(right - src_local).normalized();

			return left_dir.dot(dir_local) > right_dir.dot(dir_local);
		};
		
		std::sort(reachable_visit_points.begin(), reachable_visit_points.end(), CompareByDot);
		
		return reachable_visit_points;
	}

	static std::array<sf::Vector2i, 4> MakeRelativeDirectionsList(sf::Vector2i current) {
		return {
			current,
			{current.y, current.x},
			{-current.y, -current.x},
			-current
		};
	}

	std::vector<sf::Vector2i> BuildPath(sf::Vector2i start_position)const{

		if (!Grid.Bounds.contains(start_position)) {
			LogEnv(Error, "Start is not in the bounds");
			return {};
		}

		sf::Vector2i local_start = start_position - Grid.Bounds.getPosition();
		sf::Vector2i start_cell = Grid.LocalPositionToCellIndex(local_start);

		if(start_cell.x == -1)
			return {};

		sf::Vector2i start_coverage = GridToCoverageCell(start_cell);


		sf::Vector2i prev_coverage = start_coverage + sf::Vector2i(0, 1);
		sf::Vector2i src_coverage = start_coverage;

		std::vector<sf::Vector2i> path;
		path.push_back(local_start);
		
		for(;;) {
			auto src_local = path.back();

			sf::Vector2i dir_local = path.size() >= 2 ? path.back() - *(path.end()-2) : sf::Vector2i(0, -1);
			
			sf::Vector2i coverage_direction = src_coverage - prev_coverage;

			sf::Vector2i dst_coverage;
			std::vector<sf::Vector2i> points;

			for (auto candidate_coverage_direction : MakeRelativeDirectionsList(coverage_direction)) {
				dst_coverage = src_coverage + candidate_coverage_direction;

				if(!IsInBounds(dst_coverage))
					continue;

				points = GetSortedReachableVisitPointsNotVisited(src_coverage, dst_coverage, src_local, dir_local, path);
				
				if(points.size())
					break;
			}
			
			if(!points.size())
				break;
			points.resize(1);
			std::copy(points.begin(), points.end(), std::back_inserter(path));
			prev_coverage = src_coverage;
			src_coverage = dst_coverage;
		}
		
		for (auto& point : path) {
			point += Grid.Bounds.getPosition();
		}

		return path;
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

static void DrawForCell(const PathBuilder& builder, sf::RenderTarget& rt, sf::Vector2i coverage_cell, bool zone, bool full_zone, bool points, bool cell_outline) {
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

	PathBuilder builder(Grid, CoverageSize);
	
	bool debug_line_trace = false;

	if(debug_line_trace){
		for (auto cell : builder.TraceLine(StartPosition - Grid.Bounds.getPosition(), world_mouse_position - Grid.Bounds.getPosition())) {
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


		auto coverage_cell = builder.GridToCoverageCell(cell);

		DrawForCell(builder, rt, coverage_cell, zone, full_zone, points, cell_outline);
	} else {
		for(int x = 0; x < builder.CoverageGridSize.x; x++){
			for(int y = 0; y < builder.CoverageGridSize.y; y++){
				sf::Vector2i coverage_cell(x, y);
				DrawForCell(builder, rt, coverage_cell, zone, full_zone, points, cell_outline);
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

	//Path = Grid.BuildPath(start_position, step);

	Path = PathBuilder(Grid, CoverageSize).BuildPath(start_position);
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
