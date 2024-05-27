#pragma once

#include "vacuum_cleaner_operator.hpp"

class EvolutionTraining {
	std::vector<VacuumCleanerOperator> m_Population;
	std::vector<VacuumCleanerOperator> m_BestOfEachGoal;
	Environment m_Env;

	sf::Vector2f m_StartingPosition{0, 0};

	size_t m_Generation = 0;
	size_t m_IterationsNumber = 0;

	size_t m_HighestGoal = 0;
	float m_HighestFitness = -2;
public:
	EvolutionTraining(std::optional<std::string> filepath);

	void Tick(float dt);

	void NextGeneration();

	std::vector<VacuumCleanerOperator> PopulationFromSorted(const std::vector<VacuumCleanerOperator> &population);

	void SortPopulation();

	void Draw(sf::RenderTarget& rt, bool debug);

	void DrawUI(sf::RenderTarget& rt);

	void SaveBest();

	static std::string MakePath(size_t goal, size_t index);
};
