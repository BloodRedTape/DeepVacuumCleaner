#include "agent.hpp"

void VacuumCleanerAgent::Update(float dt, VacuumCleaner& cleaner, const Environment& env){
	if (!env.IsFullfiled()) 
		return;

	if(m_CurrentGoal >= env.Path.size())
		return;

	auto [forward, rotation] = Iterate(cleaner, env);

	sf::Vector2f goal(env.Path[m_CurrentGoal]);

	cleaner.Move(forward * dt, rotation * dt);

	if ((cleaner.Position - goal).length() < CleanerRadius / 2.f) 
		m_CurrentGoal++;
}

int VacuumCleanerAgent::Goal()const {
	return m_CurrentGoal;
}

void VacuumCleanerAgent::Reset() {
	m_CurrentGoal = 0;
}

bool VacuumCleanerAgent::HasFinished(const VacuumCleaner &cleaner, const Environment& env)const {
	if(env.Path.empty())
		return true;

	sf::Vector2f goal(env.Path.back());

		
	return (cleaner.Position - goal).length() < CleanerRadius / 2.f;
}
