#include "stupid_agent.hpp"
#include "utils/math.hpp"

sf::Vector2f StupidAgent::Iterate(const VacuumCleaner& cleaner, const Environment& env)const {
	return StaticIterate(cleaner, m_CurrentGoal, env);
}

sf::Vector2f StupidAgent::StaticIterate(const VacuumCleaner &cleaner, size_t current_goal, const Environment &env) {
	sf::Vector2f goal(env.Path[current_goal]);

	auto direction = goal - cleaner.Position;
	
	int ForwardSpeed = CleanerSpeed.x;
	int RotationSpeed = CleanerSpeed.y;

	auto angle = Math::AngleSigned(cleaner.Direction(), direction);
	float forward = ForwardSpeed;
	float rotation = Math::Sign(angle) * RotationSpeed;

	if (std::abs(angle) > 5)
		return {((std::abs(angle) / 180.f) * ForwardSpeed), rotation};
	
	return {forward, rotation};
}