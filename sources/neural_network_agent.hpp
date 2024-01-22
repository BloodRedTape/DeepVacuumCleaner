#pragma once

#include "nn.hpp"
#include <vector>
#include "vacuum_cleaner.hpp"
#include "environment.hpp"

class NeuralNetworkAgent {
	NeuralNetwork m_NN;
	int m_CurrentGoal = 0;
	std::vector<std::pair<float, size_t>> m_NearsetsToGoal;
	
	float m_DistanceFromStartToGoal = 0.f;

	float m_AvgRotationDifference = 0.f;

	bool m_HasEscaped = false;
	size_t m_Iteration = 0;
public:

	NeuralNetworkAgent(NeuralNetworkAgent &&) = default;
	NeuralNetworkAgent(const NeuralNetworkAgent &) = default;

	NeuralNetworkAgent &operator=(NeuralNetworkAgent &&) = default;
	NeuralNetworkAgent &operator=(const NeuralNetworkAgent &) = default;

	NeuralNetworkAgent(NeuralNetwork &&nn);

	NeuralNetworkAgent(int num_sensors);

	sf::Vector2f Iterate(const VacuumCleaner &cleaner, const Environment & env, size_t it);

	size_t CurrentGoal()const;

	void Reset(float rotation = 0);

	bool HasEscaped()const;

	float AvgNearesstToGoal()const;

	size_t AvgFastestToGoal()const;

	float FitnessFunction(const VacuumCleaner &cleaner, const Environment& env)const;

	void SaveToFile(const std::string& filename);

	void LoadFromFile(const std::string& filename);

	static NeuralNetworkAgent Crossover(const NeuralNetworkAgent& first, const NeuralNetworkAgent &second);

	static NeuralNetworkAgent Mutate(const NeuralNetworkAgent& agent, float rate);
};