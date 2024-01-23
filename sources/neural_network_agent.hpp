#pragma once

#include "nn.hpp"
#include <vector>
#include "vacuum_cleaner.hpp"
#include "environment.hpp"

class NeuralNetworkAgent {
	NeuralNetwork m_NN;
	int m_CurrentGoal = 0;
	bool m_HasEscaped = false;
	size_t m_Iteration = 0;
	float m_TotalDistanceTraveled = 0.f;

	float m_InitialDistanceToGoal = 0.f;
	float m_MaxDistanceToGoal = 0.f;
	float m_MinDistanceToGoal = 0.f;
	float m_CurrentDistanceToGoal = 0.f;
public:

	NeuralNetworkAgent(NeuralNetworkAgent &&) = default;
	NeuralNetworkAgent(const NeuralNetworkAgent &) = default;

	NeuralNetworkAgent &operator=(NeuralNetworkAgent &&) = default;
	NeuralNetworkAgent &operator=(const NeuralNetworkAgent &) = default;

	NeuralNetworkAgent(NeuralNetwork &&nn);

	NeuralNetworkAgent(int num_sensors);

	void Reset(const VacuumCleaner &cleaner, const Environment &env);

	void BeginGoal(size_t goal, const VacuumCleaner &cleaner, const Environment &env);

	sf::Vector2f Iterate(const VacuumCleaner &cleaner, const Environment & env, size_t it);

	size_t CurrentGoal()const;

	bool HasNotTraveled()const;

	bool TooFarGone()const {
		return m_CurrentDistanceToGoal / m_InitialDistanceToGoal > 2;
	}

	float CurrentDistanceToGoalNormalized()const{
		return m_CurrentDistanceToGoal / m_InitialDistanceToGoal;	
	}

	float FitnessFunction(const VacuumCleaner &cleaner, const Environment& env)const;

	void SaveToFile(const std::string& filename);

	void LoadFromFile(const std::string& filename);

	static NeuralNetworkAgent Crossover(const NeuralNetworkAgent& first, const NeuralNetworkAgent &second);

	static NeuralNetworkAgent Mutate(const NeuralNetworkAgent& agent, float chance, float range);
};