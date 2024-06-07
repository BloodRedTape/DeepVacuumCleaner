#pragma once

#include <SFML/Graphics.hpp>
#include <optional>
#include <vector>
#include <fstream>
#include "bsl/serialization_std.hpp"
#include "env/environment.hpp"

struct Sensor {
	float Rotation = 0.f;
};

struct VacuumCleanerState{
	float DistanceToGoal = 0.f;
	float RotationToGoal = 0.f;
	std::vector<float> SensorsData;
};

template<>
struct Serializer<VacuumCleanerState>{
	static void ToStream(const VacuumCleanerState& state, std::ostream& stream) {
		Serializer<float>::ToStream(state.DistanceToGoal, stream);
		Serializer<float>::ToStream(state.RotationToGoal, stream);
		Serializer<std::vector<float>>::ToStream(state.SensorsData, stream);
	}

	static std::optional<VacuumCleanerState> FromStream(std::istream& stream) {
		auto d = Serializer<float>::FromStream(stream);
		auto r = Serializer<float>::FromStream(stream);
		auto s = Serializer<std::vector<float>>::FromStream(stream);

		if(!d.has_value() || !r.has_value() || !s.has_value())
			return std::nullopt;
		
		return VacuumCleanerState{d.value(), r.value(), std::move(s.value())};
	}
};

struct VacuumCleaner {
	sf::Vector2f Position{0.f, 0.f};
	float Rotation = 0.f;

	std::vector<Sensor> Sensors;

	VacuumCleaner();

	void Move(float forward, float rotate);

	void Draw(sf::RenderTarget& rt, sf::Color color = sf::Color(50, 50, 50))const;

	sf::Vector2f Direction()const;

	VacuumCleanerState GetState(std::size_t current_goal, const Environment &env)const;

	void DrawIntersections(sf::RenderTarget& rt, const Environment &env);
};