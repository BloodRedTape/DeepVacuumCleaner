#include "vacuum_cleaner.hpp"
#include "math.hpp"
#include "render.hpp"

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
	sf::CircleShape shape(Radius);

	shape.setPosition(Position);
	shape.setFillColor(color);

	shape.setOutlineColor(sf::Color::White);
	shape.setOutlineThickness(3);
	shape.setOrigin({Radius, Radius});

	rt.draw(shape);

	
	for (auto sensor : Sensors) {
		auto sensor_direction = Math::RotationToDirection(sensor.Rotation + Rotation);
		auto start = Position + sensor_direction * Radius;
		auto end = start + sensor_direction * RayLength;
		Render::DrawLine(start, end, 3, rt, sf::Color::Red);
	}

	auto start = Position + Direction() * Radius;
	Render::DrawLine(start, start + Direction() * RayLength, 3, rt, sf::Color::Green);
}

sf::Vector2f VacuumCleaner::Direction()const {
	return Math::RotationToDirection(Rotation);
}

void VacuumCleaner::DrawIntersections(sf::RenderTarget& rt, const Environment &env) {
	const auto &cleaner = *this;

	for (int i = 0; i < cleaner.Sensors.size(); i++) {
		const auto &sensor = cleaner.Sensors[i];

		auto direction = Math::RotationToDirection(cleaner.Rotation + sensor.Rotation);
		auto start = cleaner.Position + cleaner.Radius * direction; 

		float intersect = Wall::TraceNearestObstacle(start, direction, env.Walls);
			
		float radius = 5;
		sf::CircleShape shape(radius);
		shape.setFillColor(sf::Color::Red);
		shape.setPosition(start + direction * intersect);
		shape.setOrigin({radius, radius});
		rt.draw(shape);
	}
}
