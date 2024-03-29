#pragma once

#include <SFML/Graphics.hpp>
#include <optional>
#include <vector>
#include <fstream>
#include "serialization.hpp"
#include "environment.hpp"

struct Sensor {
	float Rotation = 0.f;
};

struct VacuumCleaner {
	sf::Vector2f Position{0.f, 0.f};
	float Rotation = 0.f;

	std::vector<Sensor> Sensors;

	static constexpr float Radius = 60.f;

	static constexpr float RayLength = 40;


	VacuumCleaner();

	void Move(float forward, float rotate);

	void Draw(sf::RenderTarget& rt, sf::Color color = sf::Color(50, 50, 50))const;

	sf::Vector2f Direction()const;

	void DrawIntersections(sf::RenderTarget& rt, const Environment &env);
};