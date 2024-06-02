#include "neural_network_agent.hpp"
#include "utils/math.hpp"
#include "stupid_agent.hpp"
#include "config.hpp"

NeuralNetworkAgent::NeuralNetworkAgent(NeuralNetwork &&nn):
	m_NN(std::move(nn))
{}

NeuralNetworkAgent::NeuralNetworkAgent(int num_sensors):
	m_NN(
		{num_sensors + 3, 32, 20, 10, 2},
		{"None", "Tanh", "None", "Tanh", "Tanh"}
	)
{}

sf::Vector2f NeuralNetworkAgent::Iterate(const VacuumCleaner &cleaner, const Environment & env, size_t it) {
	if (!env.IsFullfiled()) 
		return {};

	if(m_CurrentGoal >= env.Path.size())
		return {};

	m_Iteration = it;

	sf::Vector2f goal(env.Path[m_CurrentGoal]);

	auto direction = goal - cleaner.Position;
	
	float difference_with_goal = acos(direction.normalized().dot(cleaner.Direction().normalized())) / 3.14 * 180;
		
	Matrix<float> input(1, cleaner.Sensors.size() + 3);

	for (int i = 0; i < cleaner.Sensors.size(); i++) {
		const auto &sensor = cleaner.Sensors[i];

		auto direction = Math::RotationToDirection(cleaner.Rotation + sensor.Rotation);
		auto start = cleaner.Position + CleanerRadius * direction; 

		input[0][i] = Wall::TraceNearestObstacle(start, direction, env.Walls);
	}

	input[0][cleaner.Sensors.size()    ] = 0.f;//cleaner.Rotation;
	input[0][cleaner.Sensors.size() + 1] = difference_with_goal;
	input[0][cleaner.Sensors.size() + 2] = direction.length();

	auto output = m_NN.Do(input);

	float forward  = output[0][0];
	float rotation = output[0][1];

	float distance_to_goal = (goal - cleaner.Position).length();

	m_TotalDistanceTraveled += forward;
	m_CurrentDistanceToGoal = distance_to_goal;


	m_MinDistanceToGoal = std::min(m_MinDistanceToGoal, distance_to_goal);
	m_MaxDistanceToGoal = std::max(m_MaxDistanceToGoal, distance_to_goal);


	if (distance_to_goal < CleanerRadius)
		BeginGoal(m_CurrentGoal + 1, cleaner, env);

	return {forward, rotation};
}

size_t NeuralNetworkAgent::CurrentGoal()const {
	return m_CurrentGoal;
}

void NeuralNetworkAgent::Reset(const VacuumCleaner &cleaner, const Environment &env) {
	BeginGoal(0, cleaner, env);
}

bool NeuralNetworkAgent::HasNotTraveled()const {
	return std::abs(m_TotalDistanceTraveled) < Eps;
}

void NeuralNetworkAgent::BeginGoal(size_t index, const VacuumCleaner &cleaner, const Environment &env){
	m_CurrentGoal = index;

	sf::Vector2f goal(env.Path[index]);
	
	m_InitialDistanceToGoal = (cleaner.Position - goal).length();
	m_MaxDistanceToGoal = 0.f;
	m_MinDistanceToGoal = m_InitialDistanceToGoal;
}

float NeuralNetworkAgent::FitnessFunction(const VacuumCleaner &cleaner, const Environment& env)const {
	if (!env.IsFullfiled()) 
		return -1;

	if(m_CurrentGoal >= env.Path.size())
		return -1;

		
	sf::Vector2f goal(env.Path[m_CurrentGoal]);

	float normalized_min = m_MinDistanceToGoal / m_InitialDistanceToGoal;
	float normalized_max = m_MaxDistanceToGoal / m_InitialDistanceToGoal;

	float penalty = -normalized_max;
	float reward = 1.f - normalized_min;

	return m_CurrentGoal + penalty + reward;
}

void NeuralNetworkAgent::SaveToFile(const std::string& filename) {
	std::filesystem::create_directories(std::filesystem::path(filename).remove_filename());

	std::fstream file(filename, std::ios::binary | std::ios::out | std::ios::trunc);

	assert(file.is_open());

	Serializer<NeuralNetwork>::ToStream(m_NN, file);
}

void NeuralNetworkAgent::LoadFromFile(const std::string& filename) {
	std::fstream file(filename, std::ios::binary | std::ios::in);

	assert(file.is_open());

	auto nn = Serializer<NeuralNetwork>::FromStream(file);

	if(!nn.has_value())
		return;

	m_NN = std::move(nn.value());
}

NeuralNetworkAgent NeuralNetworkAgent::Crossover(const NeuralNetworkAgent& first, const NeuralNetworkAgent &second) {
	return {NeuralNetwork::Crossover(first.m_NN, second.m_NN)};
}

NeuralNetworkAgent NeuralNetworkAgent::Mutate(const NeuralNetworkAgent& agent, float chance, float range) {
	return {NeuralNetwork::MutateNetwork(agent.m_NN, chance, range)};
}