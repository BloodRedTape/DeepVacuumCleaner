#include "stupid_agent.hpp"

void StupidAgent::Update(float dt, VacuumCleaner& cleaner, const Environment& env) {
	if (!env.IsFullfiled()) 
		return;

	
	if(m_CurrentGoal >= env.Path.size())
		return;


	auto [forward, rotation] = Stuff(cleaner, m_CurrentGoal, env);

	sf::Vector2f goal(env.Path[m_CurrentGoal]);

	cleaner.Move(forward, rotation);

	if ((cleaner.Position - goal).length() < 4) 
		m_CurrentGoal++;
}

sf::Vector2f StupidAgent::Stuff(const VacuumCleaner &cleaner, size_t current_goal, const Environment &env) {
	sf::Vector2f goal(env.Path[current_goal]);

	auto direction = goal - cleaner.Position;

	float forward = 60;
	float rotation = (direction.angle() - cleaner.Direction().angle()).asDegrees();
	
	return {forward, rotation};
}