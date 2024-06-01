#pragma once

#include "neural_network_agent.hpp"
#include "vacuum_cleaner.hpp"

class VacuumCleanerOperator {
	VacuumCleaner m_Cleaner;
	NeuralNetworkAgent m_Agent{(int)m_Cleaner.Sensors.size()};

	size_t m_StandStill = 0;
	size_t m_NumberFailure = 0;
public:

	VacuumCleanerOperator() = default;

	VacuumCleanerOperator(VacuumCleanerOperator &&) = default;
	VacuumCleanerOperator(const VacuumCleanerOperator &) = default;

	VacuumCleanerOperator &operator=(VacuumCleanerOperator &&) = default;
	VacuumCleanerOperator &operator=(const VacuumCleanerOperator &) = default;

	VacuumCleanerOperator(NeuralNetworkAgent &&agent);

	VacuumCleanerOperator(const NeuralNetworkAgent &agent);
	
	void Iterate(const Environment &env, float dt, size_t it_num);

	void Draw(sf::RenderTarget& rt)const;

	void DrawFitness(sf::RenderTarget& rt, const Environment &env)const;

	void Reset(const Environment &env, sf::Vector2f position, float rotation = 0);

	float FitnessFunction(const Environment &env)const;

	size_t StandStill()const;

	size_t NumberFailure()const;
	
	size_t CurrentGoal()const;

	NeuralNetworkAgent& Agent();

	bool HasCrashed(const Environment &env)const;

	static VacuumCleanerOperator Crossover(const VacuumCleanerOperator& first, const VacuumCleanerOperator &second);

	static VacuumCleanerOperator Mutate(const VacuumCleanerOperator& agent, float chance, float range);
};
