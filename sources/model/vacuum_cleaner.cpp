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

void VacuumCleaner::DrawIntersections(sf::RenderTarget& rt, const Environment &env) {
	const auto &cleaner = *this;

	for (int i = 0; i < cleaner.Sensors.size(); i++) {
		const auto &sensor = cleaner.Sensors[i];

		auto direction = Math::RotationToDirection(cleaner.Rotation + sensor.Rotation);
		auto start = cleaner.Position + CleanerRadius * direction; 

		float intersect = Wall::TraceNearestObstacle(start, direction, env.Walls);
			
		const float radius = 5;

		Render::DrawCircle(rt, start + direction * intersect, radius, sf::Color::Red);
	}
}
