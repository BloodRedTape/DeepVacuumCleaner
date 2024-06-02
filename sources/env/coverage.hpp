#pragma once

#include <optional>
#include "env/grid.hpp"

struct CoverageDecomposition {
	GridDecomposition &Grid;
	std::size_t CoverageSize = 3;
	sf::Vector2i CoverageGridSize;
	std::vector<sf::Vector2i> VisitPoints;

	CoverageDecomposition(GridDecomposition &grid, std::size_t coverage_size = 3);

	void Rebuild();

	sf::Vector2i GridToCoverageCell(sf::Vector2i grid_index)const;

	sf::IntRect CoverageCellRect(sf::Vector2i coverage_index)const;

	sf::IntRect CellToLocalRect(sf::IntRect rect)const;

	sf::IntRect CoverageCellLocalRect(sf::Vector2i coverage_index)const;

	bool IsInBounds(sf::Vector2i coverage_index)const;

	sf::IntRect Extend(sf::IntRect rect, int direction, int axis)const;

	bool TryExtendInCoverageCell(sf::IntRect& rect, int direction, int axis)const;

	sf::IntRect ExtendAsMuchInCoverageCell(sf::IntRect rect)const;

	std::optional<sf::IntRect> MakeZoneFrom(sf::Vector2i point)const;

	std::vector<sf::IntRect> MakeZoneDecomposition(sf::Vector2i coverage_cell)const;

	bool TryExtendUntilFullCoverage(sf::IntRect& rect, int direction, int axis)const;

	std::optional<sf::IntRect> TryExtendUntilFullCoverage(sf::IntRect rect)const;

	std::vector<sf::IntRect> ToFullCoverageZones(const std::vector<sf::IntRect> &zones)const;

	//Visit points are in world_local
	std::vector<sf::Vector2i> MakeVisitPoints(sf::Vector2i coverage_cell)const;

	std::vector<sf::Vector2i> MakeVisitPoints()const;

	std::vector<sf::Vector2i> GatherCoverageVisitPoints(sf::Vector2i coverage_cell)const;

	bool HasAnyOccupied(sf::Vector2i coverage)const;

	std::vector<sf::Vector2i> GatherCoverageVisitPointsInRadius(sf::Vector2i coverage_cell, sf::Vector2i dims = {1, 1})const;

	std::vector<sf::Vector2i> TraceLine(sf::Vector2i local_src, sf::Vector2i local_dst)const;
	
	bool AreDirectlyReachable(sf::Vector2i local_src, sf::Vector2i local_dst)const;

	std::vector<sf::Vector2i> GetSortedReachableVisitPointsNotVisited(sf::Vector2i src_coverage, sf::Vector2i dst_coverage, sf::Vector2i src_local, sf::Vector2i dir_local, const std::vector<sf::Vector2i> &visited)const;

	static std::array<sf::Vector2i, 4> MakeRelativeDirectionsList(sf::Vector2i current);

	std::vector<sf::Vector2i> BuildPath(sf::Vector2i start_position)const;
};

