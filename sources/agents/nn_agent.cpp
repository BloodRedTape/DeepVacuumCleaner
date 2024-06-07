#include "nn_agent.hpp"
#include "utils/math.hpp"

sf::Vector2f NNAgent::Iterate(const VacuumCleaner &cleaner, const Environment & env)const {
	if (!(Goal() < env.Path.size()))
		return {};

	VacuumCleanerState state = cleaner.GetState(m_CurrentGoal, env);

	auto move = NeuralNetworkAgent::MoveFromMatrix(
		m_NN.Do(
			NeuralNetworkAgent::StateToMatrix(state)
		)
	);

	float forward  = move.x;
	float rotation = move.y;

	return {forward, rotation};
}

