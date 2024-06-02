#pragma once

#include "vacuum_cleaner_operator.hpp"

class EvolutionTraining {
	std::vector<VacuumCleanerOperator> m_Population;
	std::vector<NeuralNetworkAgent> m_BestOfEachGoal;
	Environment m_Env;

	size_t m_Generation = 0;
	size_t m_IterationsNumber = 0;

	size_t m_HighestGoal = 0;
	float m_HighestFitness = -2;

	std::string m_BestPath;
public:
	EvolutionTraining(const std::string &best_path, const std::string &map_path);

	void Tick(float dt);

	void NextGeneration();

	std::vector<VacuumCleanerOperator> PopulationFromSorted(const std::vector<VacuumCleanerOperator> &population);

	void SortPopulation();

	void Draw(sf::RenderTarget& rt, bool debug);

	void DrawUI(sf::RenderTarget& rt);

	void SaveBest(const std::string &filename)const;

	void SaveBest()const{ SaveBest(m_BestPath); }

	static std::string MakePath(size_t goal, size_t index);
};
