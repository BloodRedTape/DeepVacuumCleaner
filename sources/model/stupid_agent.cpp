#include "stupid_agent.hpp"
#include "utils/math.hpp"

void StupidAgent::Update(float dt, VacuumCleaner& cleaner, const Environment& env) {
	if (!env.IsFullfiled()) 
		return;

	if(m_CurrentGoal >= env.Path.size())
		return;

	auto [forward, rotation] = Stuff(cleaner, m_CurrentGoal, env);

	sf::Vector2f goal(env.Path[m_CurrentGoal]);

	cleaner.Move(forward * dt, rotation * dt);

	if ((cleaner.Position - goal).length() < CleanerRadius / 2.f) 
		m_CurrentGoal++;
}

sf::Vector2f StupidAgent::Stuff(const VacuumCleaner &cleaner, size_t current_goal, const Environment &env) {
	sf::Vector2f goal(env.Path[current_goal]);

	auto direction = goal - cleaner.Position;

	int RotationSpeed = 60;
	int ForwardSpeed = 60;

	auto angle = Math::AngleSigned(cleaner.Direction(), direction);
	float forward = ForwardSpeed;
	float rotation = Math::Sign(angle) * RotationSpeed;

	if (std::abs(angle) > 5)
		return {((std::abs(angle) / 180.f) * ForwardSpeed), rotation};
	
	return {forward, rotation};
}