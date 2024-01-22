#pragma once

#include "vacuum_cleaner.hpp"
#include "environment.hpp"

class StupidAgent {
	int m_CurrentGoal = 0;
public:	
	void Update(float dt, VacuumCleaner& cleaner, const Environment& env);

	static sf::Vector2f Stuff(const VacuumCleaner &cleaner, size_t current_goal, const Environment &env);
};
