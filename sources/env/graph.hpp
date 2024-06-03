#pragma once

#include "env/coverage.hpp"
#include <unordered_map>
#include <SFML/Graphics/RenderTarget.hpp>

struct Neighbours{
	std::vector<sf::Vector2i> Neighbours;
};

class Graph {
	mutable std::unordered_map<sf::Vector2i, Neighbours> m_Vertices;
public:
	Graph() = default;

	Graph(std::unordered_map<sf::Vector2i, Neighbours> &&vertices):
		m_Vertices(std::move(vertices))
	{}

	const Neighbours &operator[](const sf::Vector2i &point)const{
		return m_Vertices[point];
	}

	void Draw(sf::RenderTarget &rt, sf::Vector2i offset = {0, 0})const;


	static Graph MakeFrom(const CoverageDecomposition &coverage);
};
