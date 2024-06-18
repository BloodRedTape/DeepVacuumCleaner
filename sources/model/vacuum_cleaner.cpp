#include "vacuum_cleaner.hpp"
#include "utils/math.hpp"
#include "utils/render.hpp"
#include "config.hpp"

VacuumCleaner::VacuumCleaner() {
	Sensors.push_back({0});

	Sensors.push_back({30});
	Sensors.push_back({-30});

	Sensors.push_back({60});
	Sensors.push_back({-60});

	Sensors.push_back({180});
}

void VacuumCleaner::Move(float forward, float rotate) {
	Rotation += rotate;
	Position += Direction() * forward;
}

void VacuumCleaner::Draw(sf::RenderTarget& rt, sf::Color color)const {

	Render::DrawCircle(rt, Position, CleanerRadius, color, 3, sf::Color::White);
	
	for (auto sensor : Sensors) {
		auto sensor_direction = Math::RotationToDirection(sensor.Rotation + Rotation);
		auto start = Position + sensor_direction * CleanerRadius;
		auto end = start + sensor_direction * CleanerRayLength;
		Render::DrawLine(rt, start, end, 3, sf::Color::Red);
	}

	auto start = Position + Direction() * CleanerRadius;
	Render::DrawLine(rt, start, start + Direction() * CleanerRayLength, 3, sf::Color::Green);
}

sf::Vector2f VacuumCleaner::Direction()const {
	return Math::RotationToDirection(Rotation);
}

VacuumCleanerSensorsState VacuumCleaner::GetSensorsState(const Environment& env)const {
	VacuumCleanerSensorsState state;

	for (auto sensor: Sensors) {
		auto direction = Math::RotationToDirection(Rotation + sensor.Rotation);
#if 0
		auto start = Position + CleanerRadius * direction; 
#else
		auto start = Position;
#endif
		auto[distance, normal] = Wall::TraceNearestObstacleWithNormal(start, direction, env.Walls);
		state.SensorsData.push_back(distance);
		state.SensorsIntersectionNormal.push_back(normal);
	}

	return state;
}


VacuumCleanerState VacuumCleaner::GetState(std::size_t current_goal, const Environment& env)const {
	VacuumCleanerState state = GetSensorsState(env);
	sf::Vector2f goal(env.Path[current_goal]);

	auto direction = goal - Position;
	
	float difference_with_goal = acos(direction.normalized().dot(Direction().normalized())) / 3.14 * 180;
	
	state = GetSensorsState(env);

	state.RotationToGoal = difference_with_goal;
	state.DistanceToGoal = direction.length();

	return state;
}

void VacuumCleaner::DrawIntersections(sf::RenderTarget& rt, const Environment &env) {
	const auto &cleaner = *this;
	
	auto state = cleaner.GetSensorsState(env);

	const float radius = 5;
	const float normal_size = 40.f;

	for (int i = 0; i < cleaner.Sensors.size(); i++) {

		auto direction = Math::RotationToDirection(cleaner.Rotation + cleaner.Sensors[i].Rotation);
		auto start = cleaner.Position; 

		float distance = state.SensorsData[i];
		sf::Vector2f normal = state.SensorsIntersectionNormal[i];
			

		auto intersection_point = start + direction * distance;

		Render::DrawCircle(rt, intersection_point, radius, sf::Color::Red);


		Render::DrawLine(rt, intersection_point, intersection_point + normal * normal_size, 4.f, sf::Color::Red);
	}
}
