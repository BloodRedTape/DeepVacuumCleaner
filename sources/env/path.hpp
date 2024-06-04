#pragma once

#include "environment.hpp"

struct PathBuilder{
	virtual std::vector<sf::Vector2i> MakePath(const Environment &env)const = 0;

	virtual std::string Name()const = 0;

	std::optional<sf::Vector2i> FindFirstUnvisited(const Environment &env, const std::vector<sf::Vector2i> &candidates, const std::vector<sf::Vector2i> &path)const;
};

struct BreadthSearchPathFinder: PathBuilder{
	std::vector<sf::Vector2i> MakePath(const Environment &env)const override;

	std::string Name()const override{return "Breadth First"; }
};

struct BreadthSearchWithSortPathFinder: PathBuilder{
	std::vector<sf::Vector2i> MakePath(const Environment &env)const override;

	std::string Name()const override{return "Breadth First With Sort"; }
};

struct FirstNearWallPathBuilder : PathBuilder {
	std::vector<sf::Vector2i> MakePath(const Environment &env)const override;

	std::string Name()const override{return "First Near Wall - Some Cringe, don't use"; }
};

struct DirectionSortPathBuilder : PathBuilder {
	std::vector<sf::Vector2i> MakePath(const Environment &env)const override;

	std::string Name()const override{return "Direction Sort"; }
};

struct RightFirstPathBuilder : PathBuilder {
	std::vector<sf::Vector2i> MakePath(const Environment &env)const override;

	std::string Name()const override{return "Right First"; }
};
