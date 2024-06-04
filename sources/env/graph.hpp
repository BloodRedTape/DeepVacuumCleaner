#pragma once

#include "env/coverage.hpp"
#include <unordered_map>
#include <SFML/Graphics/RenderTarget.hpp>

struct SortByDirection {
	sf::Vector2i Direction;

	bool operator()(sf::Vector2i l, sf::Vector2i r)const;

	static bool BeforePoint(sf::Vector2i first, sf::Vector2i second, sf::Vector2i direciton);

	static bool BeforeOrEqualPoint(sf::Vector2i first, sf::Vector2i second, sf::Vector2i direciton) {
		return BeforePoint(first, second, direciton)
			|| (!BeforePoint(first, second, direciton) && !BeforePoint(second, first, direciton));
	}
};

struct SortByDistanceTo {
	sf::Vector2i Point;

	bool operator()(sf::Vector2i l, sf::Vector2i r)const;
};

struct SortByAngleCouterClockwize{
	sf::Vector2i Forward;
	sf::Vector2i Center;

	bool operator()(sf::Vector2i l, sf::Vector2i r)const;
};

struct Neighbours{
	bool HasAnyOccupied = false;
	std::vector<sf::Vector2i> Neighbours;

	void AddUnique(sf::Vector2i neighbour);

	void AppendUnique(const sf::Vector2i *points, std::size_t count);

	void AppendUnique(const std::vector<sf::Vector2i> &points);

	void Remove(sf::Vector2i point);
};

class Graph {
	mutable std::unordered_map<sf::Vector2i, Neighbours> m_Vertices;
public:
	Graph() = default;

	Graph(std::unordered_map<sf::Vector2i, Neighbours> &&vertices):
		m_Vertices(std::move(vertices))
	{}

	const Neighbours &operator[](const sf::Vector2i &point)const{
		return At(point);
	}

	const Neighbours &At(const sf::Vector2i &point)const{
		return m_Vertices[point];
	}

	void MakeConnection(sf::Vector2i src, sf::Vector2i dst, bool is_oriented = true);

	bool IsReachable(sf::Vector2i source, sf::Vector2i dst)const;

	void Draw(sf::RenderTarget &rt, sf::Vector2i offset = {0, 0}, bool draw_directions = false)const;

	void DrawVertex(sf::RenderTarget &rt, sf::Vector2i vertex, sf::Vector2i offset = {0, 0}, bool draw_directions = false)const;
	
	std::vector<sf::Vector2i> ShortestPath(sf::Vector2i src, sf::Vector2i dst)const;

	std::size_t CountReachableFrom(sf::Vector2i src)const;

	std::size_t Size()const {
		return m_Vertices.size();
	}

	std::vector<sf::Vector2i> GetSortedNeighboursInDirection(sf::Vector2i vertex, sf::Vector2i direction)const;

	std::vector<sf::Vector2i> GetSortedNeighboursInDirection(sf::Vector2i vertex, sf::Vector2i direction, bool can_be_occupied)const;

	static Graph MakeFrom(const CoverageDecomposition &coverage);

	static Graph MakeOptimizedFrom(const CoverageDecomposition &coverage);

	static Graph MakeWall(const Graph &graph);
};
