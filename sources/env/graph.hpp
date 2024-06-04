#pragma once

#include "env/coverage.hpp"
#include <unordered_map>
#include <SFML/Graphics/RenderTarget.hpp>

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

	bool IsReachable(sf::Vector2i source, sf::Vector2i dst)const;

	void Draw(sf::RenderTarget &rt, sf::Vector2i offset = {0, 0}, bool draw_directions = false)const;


	static Graph MakeFrom(const CoverageDecomposition &coverage);

	static Graph MakeOptimizedFrom(const CoverageDecomposition &coverage);
};
