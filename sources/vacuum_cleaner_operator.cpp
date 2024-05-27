#include "vacuum_cleaner_operator.hpp"
#include "math.hpp"
#include "render.hpp"

VacuumCleanerOperator::VacuumCleanerOperator(NeuralNetworkAgent &&agent):
	m_Agent(std::move(agent))
{}


void VacuumCleanerOperator::Iterate(const Environment &env, float dt, size_t it_num) {

	bool UseDeltaTime = false;
	auto it = m_Agent.Iterate(m_Cleaner, env, it_num);
	auto [forward, rotation] = it.cwiseMul(sf::Vector2f(60, 60)) * (UseDeltaTime ? dt : 0.016f);

	if(std::abs(forward) < Eps && std::abs(rotation) < Eps)
		m_StandStill++;

	if(std::isnan(forward) || std::isinf(forward)){
		std::cout << "[" << this << "] forward Number failure\n";
		m_NumberFailure++;
	}

	if(std::isnan(rotation) || std::isinf(rotation)){
		std::cout << "[" << this << "] rotation Number failure\n";
		m_NumberFailure++;
	}

	m_Cleaner.Move(forward, rotation);
}

void VacuumCleanerOperator::Draw(sf::RenderTarget& rt)const{
	m_Cleaner.Draw(rt, Render::GetRainbowColor(m_Agent.CurrentGoal(), 7));
}

void VacuumCleanerOperator::Reset(sf::Vector2f position, float rotation) {
	m_Cleaner.Position = position;
	m_Cleaner.Rotation = rotation;
	m_Agent.Reset();
}

size_t VacuumCleanerOperator::FitnessFunction(const Environment &env)const {
	return m_Agent.FitnessFunction(m_Cleaner, env);
}

size_t VacuumCleanerOperator::StandStill()const {
	return m_StandStill;
}

size_t VacuumCleanerOperator::NumberFailure()const {
	return m_NumberFailure;
}

size_t VacuumCleanerOperator::CurrentGoal()const {
	return m_Agent.CurrentGoal();
}

float VacuumCleanerOperator::AvgNearestToGoal()const {
	return m_Agent.AvgNearesstToGoal();
}

NeuralNetworkAgent& VacuumCleanerOperator::Agent() {
	return m_Agent;
}

bool VacuumCleanerOperator::HasCrashed(const Environment &env)const {
	for (const auto &wall : env.Walls) {
		if(Math::LineCircleIntersection(sf::Vector2f(wall.Start), sf::Vector2f(wall.End), m_Cleaner.Position, m_Cleaner.Radius))
			return true;
	}
	return false;
}

VacuumCleanerOperator VacuumCleanerOperator::Crossover(const VacuumCleanerOperator& first, const VacuumCleanerOperator &second) {
	return {NeuralNetworkAgent::Crossover(first.m_Agent, second.m_Agent)};
}

VacuumCleanerOperator VacuumCleanerOperator::Mutate(const VacuumCleanerOperator& agent, float rate) {
	return {NeuralNetworkAgent::Mutate(agent.m_Agent, rate)};
}

