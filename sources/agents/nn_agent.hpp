#pragma once

#include "model/vacuum_cleaner.hpp"
#include "env/environment.hpp"
#include "agents/agent.hpp"
#include "model/neural_network_agent.hpp"

class NNAgent: public VacuumCleanerAgent{
	NeuralNetwork m_NN;

	mutable sf::Vector2f m_Trench;
	mutable sf::Vector2f m_OffsetedTrench;
public:	
	NNAgent(NeuralNetwork &&nn = {});

	std::string Name()const override{ return "Neural Network Agent"; }

	sf::Vector2f Iterate(const VacuumCleaner &cleaner, const Environment & env)const override;

	void DrawDebugData(sf::RenderTarget &rt)override;
};
