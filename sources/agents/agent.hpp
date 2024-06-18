#pragma once

#include "model/vacuum_cleaner.hpp"

class VacuumCleanerAgent {
protected:
	int m_CurrentGoal = 0;
public:
	virtual sf::Vector2f Iterate(const VacuumCleaner &cleaner, const Environment & env)const = 0;

	void Update(float dt, VacuumCleaner& cleaner, const Environment& env);

	virtual int Goal()const; 

	virtual void Reset();

	virtual std::string Name()const = 0;

	virtual void DrawDebugData(sf::RenderTarget &rt){};

	bool HasFinished(const VacuumCleaner &cleaner, const Environment &env)const;
};