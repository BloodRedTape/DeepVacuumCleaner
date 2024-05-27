#include "neural_network_agent.hpp"
#include "math.hpp"
#include "stupid_agent.hpp"

NeuralNetworkAgent::NeuralNetworkAgent(NeuralNetwork &&nn):
	m_NN(std::move(nn))
{}

NeuralNetworkAgent::NeuralNetworkAgent(int num_sensors):
	m_NN(
		{num_sensors + 3, 18, 18, 8, 2},
		{"None", "None", "Sigmoid", "Sigmoid", "Tanh"}
	)
{
	Reset();
}

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
		auto start = cleaner.Position + cleaner.Radius * direction; 

		input[0][i] = Wall::TraceNearestObstacle(start, direction, env.Walls);
	}

	input[0][cleaner.Sensors.size()    ] = 0.f;//cleaner.Rotation;
	input[0][cleaner.Sensors.size() + 1] = difference_with_goal;
	input[0][cleaner.Sensors.size() + 2] = direction.length();

	auto output = m_NN.Do(input);

	float forward  = output[0][0];
	float rotation = output[0][1];

	auto [stupid_forward, stupid_rotation] = StupidAgent::Stuff(cleaner, m_CurrentGoal, env);

	m_AvgRotationDifference += std::abs(stupid_rotation - rotation);

	float length_to_goal = (cleaner.Position - goal).length();

	if(m_NearsetsToGoal.size() == m_CurrentGoal){
		m_NearsetsToGoal.push_back({9999999, 0});
		m_DistanceFromStartToGoal = length_to_goal;
	}

	m_HasEscaped = std::abs(length_to_goal - m_DistanceFromStartToGoal) > m_DistanceFromStartToGoal;

	if(m_NearsetsToGoal[m_CurrentGoal].first > length_to_goal){
		m_NearsetsToGoal[m_CurrentGoal].first  = length_to_goal;
		m_NearsetsToGoal[m_CurrentGoal].second = it;
	}

	if (length_to_goal < cleaner.Radius) 
		m_CurrentGoal++;

	return {forward, rotation};
}

size_t NeuralNetworkAgent::CurrentGoal()const {
	return m_CurrentGoal;
}

void NeuralNetworkAgent::Reset(float rotation) {
	m_CurrentGoal = 0;
}

bool NeuralNetworkAgent::HasEscaped()const {
	return m_HasEscaped;
}

float NeuralNetworkAgent::AvgNearesstToGoal()const {
	float avg = 0.f;

	for(auto [x, _]: m_NearsetsToGoal)
		avg += x;

	return avg / m_NearsetsToGoal.size();
}

size_t NeuralNetworkAgent::AvgFastestToGoal()const {
	size_t sum = 0;

	size_t avg = 0;

	for (auto [_, x] : m_NearsetsToGoal)
	{
		sum += x;
		auto it_to_goal = x - sum;
		avg += it_to_goal;
	}

	return avg / m_NearsetsToGoal.size();
}

float NeuralNetworkAgent::FitnessFunction(const VacuumCleaner &cleaner, const Environment& env)const {
	if (!env.IsFullfiled()) 
		return -1;

	if(m_CurrentGoal >= env.Path.size())
		return -1;

		
	sf::Vector2f goal(env.Path[m_CurrentGoal]);

	constexpr float GoalPrice = 100000;

	return GoalPrice * m_CurrentGoal - AvgNearesstToGoal() - AvgFastestToGoal() + (m_AvgRotationDifference / m_Iteration) * 1000;
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

NeuralNetworkAgent NeuralNetworkAgent::Mutate(const NeuralNetworkAgent& agent, float rate) {
	return {NeuralNetwork::MutateNetwork(agent.m_NN, rate)};
}