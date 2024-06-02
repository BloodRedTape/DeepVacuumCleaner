#pragma once

#include <optional>
#include <unordered_map>
#include "env/grid.hpp"

namespace std {
	template<typename T>
	struct hash<sf::Vector2<T>>{
		std::size_t operator()(const sf::Vector2<T>& vector)const{
			return std::hash<T>()(vector.x) ^ std::hash<T>()(vector.y);
		}
	};
}

struct CoverageDecomposition {
	GridDecomposition &Grid;
	std::size_t CoverageSize = 3;
	sf::Vector2i CoverageGridSize;
	std::vector<sf::Vector2i> VisitPoints;
	std::vector<sf::Vector2i> WallVisitPoints;

	mutable std::unordered_map<sf::Vector2i, std::vector<sf::Vector2i>> ProducedVisitPointsCache;
	mutable std::unordered_map<sf::Vector2i, std::vector<sf::Vector2i>> LocatedVisitPointsCache;
	mutable std::unordered_map<sf::Vector2i, std::vector<sf::IntRect>> ZoneDecompositionCache;
	mutable std::unordered_map<sf::Vector2i, std::vector<sf::IntRect>> CoverageZoneDecompositionCache;

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

	std::vector<sf::Vector2i> GatherWallsCoverageVisitPoints()const;

	bool HasAnyOccupied(sf::Vector2i coverage)const;

	std::vector<sf::Vector2i> GatherCoverageVisitPointsInRadius(sf::Vector2i coverage_cell, sf::Vector2i dims = {1, 1})const;

	std::vector<sf::Vector2i> TraceLine(sf::Vector2i local_src, sf::Vector2i local_dst)const;
	
	bool AreDirectlyReachable(sf::Vector2i local_src, sf::Vector2i local_dst)const;

	std::vector<sf::Vector2i> GetSortedReachableVisitPointsNotVisited(sf::Vector2i src_coverage, sf::Vector2i dst_coverage, sf::Vector2i src_local, sf::Vector2i dir_local, const std::vector<sf::Vector2i> &visited)const;

	static std::array<sf::Vector2i, 4> MakeRelativeDirectionsList(sf::Vector2i current);

	std::vector<sf::Vector2i> BuildPath(sf::Vector2i start_position)const;

	std::optional<std::size_t> GetNearestReachable(std::vector<sf::Vector2i> &candidates, sf::Vector2i dir_local, sf::Vector2i src_local)const;

	std::vector<sf::Vector2i> BuildPath2(sf::Vector2i start_position)const;

	std::vector<sf::Vector2i> SimplePathAlgorithm(sf::Vector2i start_position)const;
	
	template<typename PredicateType>
	void ForEachCoverage(PredicateType predicate)const{
		for (int x = 0; x < CoverageGridSize.x; x++) {
			for (int y = 0; y < CoverageGridSize.y; y++) {
				predicate(sf::Vector2i(x, y));
			}
		}
	}
};

