#pragma once

#include "model/vacuum_cleaner.hpp"
#include "env/environment.hpp"
#include "agents/agent.hpp"

class StupidAgent: public VacuumCleanerAgent{
public:	
	sf::Vector2f Iterate(const VacuumCleaner &cleaner, const Environment &env)const override;

	static sf::Vector2f StaticIterate(const VacuumCleaner &cleaner, size_t current_goal, const Environment &env);

	std::string Name()const override{ return "Naive Agent"; }
};
