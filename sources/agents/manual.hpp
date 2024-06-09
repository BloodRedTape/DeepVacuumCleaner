#pragma once

#include "agent.hpp"

class ManualAgent: public VacuumCleanerAgent{
public:
	sf::Vector2f Iterate(const VacuumCleaner &cleaner, const Environment & env)const override;

	std::string Name()const override{ return "Maunal Agent"; };
};