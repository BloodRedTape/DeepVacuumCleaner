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

struct VacuumCleanerSensorsState {
	std::vector<float> SensorsData;
	std::vector<sf::Vector2f> SensorsIntersectionNormal;

	VacuumCleanerSensorsState() = default;

	VacuumCleanerSensorsState(const std::vector<float>& sensorsData, const std::vector<sf::Vector2f>& sensorsIntersectionNormal): 
		SensorsData(sensorsData), 
		SensorsIntersectionNormal(sensorsIntersectionNormal) 
	{}

	bool IsCollided(std::size_t index)const {
		if(index >= SensorsData.size())
			return false;

		return SensorsData[index] < CleanerRadius;
	}
};

struct VacuumCleanerState: public VacuumCleanerSensorsState{
	float DistanceToGoal = 0.f;
	float RotationToGoal = 0.f;

	VacuumCleanerState() = default;

	VacuumCleanerState(const std::vector<float>& sensorsData, const std::vector<sf::Vector2f>& sensorsIntersectionNormal, float distanceToGoal, float rotationToGoal): 
		VacuumCleanerSensorsState(sensorsData, sensorsIntersectionNormal), 
		DistanceToGoal(distanceToGoal), 
		RotationToGoal(rotationToGoal)
	{}

	VacuumCleanerState(const VacuumCleanerSensorsState& state) {
		SensorsData = state.SensorsData;
		SensorsIntersectionNormal = state.SensorsIntersectionNormal;
	}
};

template<>
struct Serializer<VacuumCleanerState>{
	static void ToStream(const VacuumCleanerState& state, std::ostream& stream) {
		Serializer<std::vector<float>>::ToStream(state.SensorsData, stream);
		Serializer<std::vector<sf::Vector2f>>::ToStream(state.SensorsIntersectionNormal, stream);
		Serializer<float>::ToStream(state.DistanceToGoal, stream);
		Serializer<float>::ToStream(state.RotationToGoal, stream);
	}

	static std::optional<VacuumCleanerState> FromStream(std::istream& stream) {
		auto s = Serializer<std::vector<float>>::FromStream(stream);
		auto n = Serializer<std::vector<sf::Vector2f>>::FromStream(stream);
		auto d = Serializer<float>::FromStream(stream);
		auto r = Serializer<float>::FromStream(stream);

		if(!d.has_value() || !r.has_value() || !s.has_value() || !n.has_value())
			return std::nullopt;
		
		return VacuumCleanerState(std::move(s.value()), std::move(n.value()), d.value(), r.value());
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

	VacuumCleanerSensorsState GetSensorsState(const Environment &env)const;

	VacuumCleanerState GetState(std::size_t current_goal, const Environment &env)const;

	void DrawIntersections(sf::RenderTarget& rt, const Environment &env);
};