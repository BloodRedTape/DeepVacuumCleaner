#pragma once

#include "vacuum_cleaner.hpp"
#include "env/environment.hpp"

class StupidAgent {
	int m_CurrentGoal = 0;
public:	
	void Update(float dt, VacuumCleaner& cleaner, const Environment& env);

	static sf::Vector2f Stuff(const VacuumCleaner &cleaner, size_t current_goal, const Environment &env);

	int Goal()const{return m_CurrentGoal; }

	void Reset(){ m_CurrentGoal = 0; }
};
