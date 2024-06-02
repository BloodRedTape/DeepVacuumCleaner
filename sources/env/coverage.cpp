#include "coverage.hpp"
#include <cmath>
#include <algorithm>
#include "bsl/log.hpp"
#include "utils/math.hpp"

DEFINE_LOG_CATEGORY(Coverage)

CoverageDecomposition::CoverageDecomposition(GridDecomposition &grid, std::size_t coverage_size):
	Grid(grid),
	CoverageSize(coverage_size),
	CoverageGridSize(
		std::ceil(grid.Bounds.getSize().x / (Grid.CellSize.x * float(coverage_size))),
		std::ceil(grid.Bounds.getSize().y / (Grid.CellSize.y * float(coverage_size)))
	)
{
	Rebuild();
}

void CoverageDecomposition::Rebuild() {
	VisitPoints = MakeVisitPoints();
}

sf::Vector2i CoverageDecomposition::GridToCoverageCell(sf::Vector2i grid_index)const{
	return grid_index / int(CoverageSize);
}

sf::IntRect CoverageDecomposition::CoverageCellRect(sf::Vector2i coverage_index)const{
	return {coverage_index * int(CoverageSize), sf::Vector2i(CoverageSize, CoverageSize)};
}

sf::IntRect CoverageDecomposition::CellToLocalRect(sf::IntRect rect)const{
	return {rect.getPosition().cwiseMul(Grid.CellSize), rect.getSize().cwiseMul(Grid.CellSize)};
}

sf::IntRect CoverageDecomposition::CoverageCellLocalRect(sf::Vector2i coverage_index)const {
	return CellToLocalRect(CoverageCellRect(coverage_index));
}

bool CoverageDecomposition::IsInBounds(sf::Vector2i coverage_index)const {
	return coverage_index.x >= 0
		&& coverage_index.x < CoverageGridSize.x
		&& coverage_index.y >= 0
		&& coverage_index.y < CoverageGridSize.y;
}

sf::IntRect CoverageDecomposition::Extend(sf::IntRect rect, int direction, int axis)const {
	assert(direction == 1 || direction == -1);
	assert(axis == 0 || axis == 1);


	sf::Vector2i min = rect.getPosition();
	sf::Vector2i max = rect.getPosition() + rect.getSize();

	At(min, axis) = std::min(At(min, axis), At(min, axis) + direction);
	At(max, axis) = std::max(At(max, axis), At(max, axis) + direction);

	return {min, max - min};
}

bool CoverageDecomposition::TryExtendInCoverageCell(sf::IntRect& rect, int direction, int axis)const{
	sf::IntRect new_rect = Extend(rect, direction, axis);
	
	if(Grid.IsOccupied(new_rect))
		return false;
	
	auto coverage_cell = CoverageCellRect(GridToCoverageCell(rect.getPosition()));

	//is out of bounds
	if(!Math::IsRectInside(new_rect, coverage_cell))
		return false;
	
	rect = new_rect;
	return true;
}

sf::IntRect CoverageDecomposition::ExtendAsMuchInCoverageCell(sf::IntRect rect)const {
	while(TryExtendInCoverageCell(rect, 1, 0));
	while(TryExtendInCoverageCell(rect,-1, 0));
	while(TryExtendInCoverageCell(rect, 1, 1));
	while(TryExtendInCoverageCell(rect,-1, 1));

	return rect;
}


std::optional<sf::IntRect> CoverageDecomposition::MakeZoneFrom(sf::Vector2i point)const{
	if(Grid.IsOccupied(point))
		return std::nullopt;

	return ExtendAsMuchInCoverageCell({point, {1, 1}});
}

std::vector<sf::IntRect> CoverageDecomposition::MakeZoneDecomposition(sf::Vector2i coverage_cell)const {
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

bool CoverageDecomposition::TryExtendUntilFullCoverage(sf::IntRect& rect, int direction, int axis)const{
	sf::IntRect new_rect = Extend(rect, direction, axis);
	
	if(Grid.IsOccupied(new_rect))
		return false;
	
	if(At(new_rect.getSize(), axis) > CoverageSize)
		return false;
	
	rect = new_rect;
	return true;
}


std::optional<sf::IntRect> CoverageDecomposition::TryExtendUntilFullCoverage(sf::IntRect rect)const {
	while(TryExtendUntilFullCoverage(rect, 1, 0));
	while(TryExtendUntilFullCoverage(rect,-1, 0));
	while(TryExtendUntilFullCoverage(rect, 1, 1));
	while(TryExtendUntilFullCoverage(rect,-1, 1));

	if(rect.getSize() != sf::Vector2i(CoverageSize, CoverageSize))
		return std::nullopt;

	return rect;
}

std::vector<sf::IntRect> CoverageDecomposition::ToFullCoverageZones(const std::vector<sf::IntRect> &zones)const
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
std::vector<sf::Vector2i> CoverageDecomposition::MakeVisitPoints(sf::Vector2i coverage_cell)const{
	auto full_coverage_zones = ToFullCoverageZones(MakeZoneDecomposition(coverage_cell));

	std::vector<sf::Vector2i> points;

	for (auto zone : full_coverage_zones) {
		sf::IntRect absoule{zone.getPosition().cwiseMul(Grid.CellSize), zone.getSize().cwiseMul(Grid.CellSize)};

		points.push_back(absoule.getCenter());
	}
	return points;
}

std::vector<sf::Vector2i> CoverageDecomposition::MakeVisitPoints()const {
	std::vector<sf::Vector2i> points;
	for (int x = 0; x < CoverageGridSize.x; x++) {
		for (int y = 0; y < CoverageGridSize.y; y++) {
			auto cell_points = MakeVisitPoints({x, y});
		
			// ��� ���� points.append(cell_points)
			std::copy(cell_points.begin(), cell_points.end(), std::back_inserter(points));
		}
	}
	return points;
}

std::vector<sf::Vector2i> CoverageDecomposition::GatherCoverageVisitPoints(sf::Vector2i coverage_cell)const {
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

bool CoverageDecomposition::HasAnyOccupied(sf::Vector2i coverage)const {
	auto rect = CoverageCellRect(coverage);

	for (auto cell : Grid.OccupiedIndices) {
		if(rect.contains(cell))
			return true;
	}
	return false;
}

std::vector<sf::Vector2i> CoverageDecomposition::GatherCoverageVisitPointsInRadius(sf::Vector2i coverage_cell, sf::Vector2i dims)const {
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

std::vector<sf::Vector2i> CoverageDecomposition::TraceLine(sf::Vector2i local_src, sf::Vector2i local_dst)const {
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

bool CoverageDecomposition::AreDirectlyReachable(sf::Vector2i local_src, sf::Vector2i local_dst)const {
	for (auto line_cell : TraceLine(local_src, local_dst)) {
		
		auto coverage = TryExtendUntilFullCoverage({line_cell, {1, 1}});

		if(!coverage.has_value())
			return false;
	}
	
	return true;
}



std::vector<sf::Vector2i> CoverageDecomposition::GetSortedReachableVisitPointsNotVisited(sf::Vector2i src_coverage, sf::Vector2i dst_coverage, sf::Vector2i src_local, sf::Vector2i dir_local, const std::vector<sf::Vector2i> &visited)const{
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

std::array<sf::Vector2i, 4> CoverageDecomposition::MakeRelativeDirectionsList(sf::Vector2i current) {
	return {
		current,
		{current.y, current.x},
		{-current.y, -current.x},
		-current
	};
}

std::vector<sf::Vector2i> CoverageDecomposition::BuildPath(sf::Vector2i start_position)const{

	if (!Grid.Bounds.contains(start_position)) {
		LogCoverage(Error, "Start is not in the bounds");
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