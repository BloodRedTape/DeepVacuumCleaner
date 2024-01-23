#include "environment.hpp"
#include "math.hpp"
#include "serialization.hpp"
#include "render.hpp"
#include <fstream>
#include <SFML/Graphics.hpp>
#include "config.hpp"

float Wall::TraceNearestObstacle(sf::Vector2f position, sf::Vector2f direction, const std::vector<Wall> &walls){
	std::optional<float> nearest;
	
	for (const auto &wall : walls) {
		auto result = Math::RayLineIntersection(position, direction, sf::Vector2f(wall.Start), sf::Vector2f(wall.End));

		if(!nearest.has_value()){
			nearest = result;
			continue;
		}

		if (result.has_value() && result.value() < nearest.value()) {
			nearest = result;
		}
	}
	
	return nearest.has_value() ? nearest.value() : 9999999999;
}

bool Environment::IsFullfiled()const {
	return Path.size() > 0;
}

void Environment::Draw(sf::RenderTarget& rt) {
	for (int i = 0; i<Path.size(); i++) {
		auto point = Path[i];
		sf::CircleShape shape(PointRadius);
		shape.setPosition((sf::Vector2f)point);
		shape.setOrigin({PointRadius, PointRadius});
		rt.draw(shape);

		Render::DrawString(sf::Vector2f(point) + sf::Vector2f(0, PointRadius), std::to_string(i), rt);
	}


	for (const auto &wall : Walls) {
		Render::DrawLine(sf::Vector2f(wall.Start), sf::Vector2f(wall.End), WallHeight, rt);
	}
}

void Environment::SaveToFile(const std::string& filename) {
	std::fstream file(filename, std::ios::binary | std::ios::out);

	assert(file.is_open());

	Serialization::ToStream(Path, file);
	Serialization::ToStream(Walls, file);
}

void Environment::LoadFromFile(const std::string& filename) {
	std::fstream file(filename, std::ios::binary | std::ios::in);

	assert(file.is_open());

	Path = Serialization::FromStream<std::vector<sf::Vector2i>>(file);
	Walls = Serialization::FromStream<std::vector<Wall>>(file);
}