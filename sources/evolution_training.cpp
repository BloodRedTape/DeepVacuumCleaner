#include "evolution_training.hpp"
#include "vacuum_cleaner_operator.hpp"
#include "render.hpp"
#include "random.hpp"
#include "math.hpp"
#include "bsl/format.hpp"
#include "bsl/defer.hpp"
#include "config.hpp"

EvolutionTraining::EvolutionTraining(const std::string &best_path, const std::string &map_path):
	m_BestPath(best_path)
{
	m_Env.LoadFromFile(map_path);

	defer{
		if (!m_Population.size()) {
			for (int i = 0; i < 50; i++) {
				m_Population.push_back({});
			}
		}

		for (auto& p : m_Population) {
			p.Reset(m_Env, (sf::Vector2f)m_Env.StartPosition);
		}
	};
	
	std::fstream best(best_path, std::ios::binary | std::ios::in);
	
	if (!best.is_open()) {
		Println("Can't open '%' for reading", best_path);
		return;
	}

	m_Generation		= Serializer<std::size_t>::FromStream(best).value_or(0);
	m_IterationsNumber	= Serializer<std::size_t>::FromStream(best).value_or(0);
	m_HighestGoal		= Serializer<std::size_t>::FromStream(best).value_or(0);
	m_HighestFitness	= Serializer<float>		 ::FromStream(best).value_or(0);
	auto agents			= Serializer<std::vector<NeuralNetworkAgent>>::FromStream(best);

	if (agents.has_value()) {
		Println("Loaded % agents", agents.value().size());

		m_BestOfEachGoal = std::move(agents.value());

		for(const NeuralNetworkAgent &agent: m_BestOfEachGoal){
			m_Population.push_back(agent);
			m_Population.back().Iterate(m_Env, 0, 0);
		}
		
		m_Population = PopulationFromSorted(m_Population);
	}
}

void EvolutionTraining::Tick(float dt) {
	m_IterationsNumber++;

	SortPopulation();
	auto best_fitness = m_Population.front().FitnessFunction(m_Env);

	auto h = std::max_element(m_Population.begin(), m_Population.end(), [](auto &l, auto &r){
		return l.CurrentGoal() < r.CurrentGoal();
	});

	if(h->CurrentGoal() > m_HighestGoal) {
		m_HighestGoal = h->CurrentGoal();
		m_BestOfEachGoal.push_back(h->Agent());
		SaveBest(m_BestPath);
	}

	if(best_fitness > m_HighestFitness){
		m_HighestFitness = best_fitness;
		if(h != m_Population.begin())
			m_BestOfEachGoal.push_back(m_Population.front().Agent());
		SaveBest(m_BestPath);
	}


	std::vector<int> die;
	
	for (int i = 0; i<m_Population.size(); i++) {
		VacuumCleanerOperator &agent = m_Population[i];

		agent.Iterate(m_Env, dt, m_IterationsNumber);

		if(agent.StandStill() > StandStillToDie || agent.NumberFailure() || agent.HasCrashed(m_Env) || agent.Agent().HasNotTraveled() || agent.Agent().TooFarGone())
			die.push_back(i);
	}

	if (die.size() > 2 && die.size() + 2 >= m_Population.size()) {
		die.pop_back();
		die.pop_back();
	}

	for (int i = die.size() - 1; i>=0; i--) {
		if(m_Population.size() <= 2)
			break;

		int index = die[i];
		std::swap(m_Population[index], m_Population.back());
		m_Population.pop_back();
	}

	if (m_Population.size() == 2 || m_IterationsNumber % ((m_HighestGoal + 1) * IterationsPerGoal) == 0) {
		NextGeneration();
	}
}

void EvolutionTraining::NextGeneration() {
	assert(m_Population.size() >= 2);

	m_Generation++;
	
	Println("New Population");
	Println("BestOfEachGoal: %", m_BestOfEachGoal.size());
	
	for (const auto &best : m_BestOfEachGoal) {
		m_Population.push_back(best);
		m_Population.back().Iterate(m_Env, 0, 0);
	}
	std::copy(m_BestOfEachGoal.begin(), m_BestOfEachGoal.end(), std::back_inserter(m_Population));

	SortPopulation();

	assert(m_Population[0].FitnessFunction(m_Env) >= m_Population[1].FitnessFunction(m_Env));

	auto new_population = PopulationFromSorted(m_Population);

	std::cout << "New Generation: " << m_Generation << " size " << new_population.size() << " from " << m_Population.size() << "\n";

	m_Population = std::move(new_population);

	for (auto& p : m_Population)
		p.Reset(m_Env, sf::Vector2f(m_Env.StartPosition));
}

void EvolutionTraining::SortPopulation() {
	std::sort(m_Population.begin(), m_Population.end(), [&](auto &left, auto &right) {
		return left.FitnessFunction(m_Env) > right.FitnessFunction(m_Env);
	});
}

std::vector<VacuumCleanerOperator> EvolutionTraining::PopulationFromSorted(const std::vector<VacuumCleanerOperator> &population) {
	std::vector<VacuumCleanerOperator> new_population;

	auto count = std::min(ModelsToCrossover, population.size());
	for (int i = 0; i < count; i++) {
		for (int j = 0; j < count; j++) {
			new_population.push_back(VacuumCleanerOperator::Crossover(population[i], population[j]));
		}
	}

	for (auto& op : new_population) {
		if (GetRandom<float>(0, 1) < GenofondMutationChance)
			op = VacuumCleanerOperator::Mutate(op, MutationChance, MutationRange);
		op.Reset(m_Env, sf::Vector2f(m_Env.StartPosition));
	}

	for(int i = 0; i<count; i++){
		new_population.push_back(population[i]);
	}

	for (int i = 0; i < RandomCrossovers; i++) {
		new_population.push_back(VacuumCleanerOperator::Crossover({}, new_population[rand() % new_population.size()]));
	}

	for (int i = 0; i < std::min(BestModelsToMutate, population.size()); i++) {
		for(int j = 0; j<BestModelsMutateTimes; j++){
			new_population.push_back(VacuumCleanerOperator::Mutate(population[i], BestModelsToMutationChance, BestModelsToMutationRange));
		}
	}
	
	return new_population;
}

void EvolutionTraining::Draw(sf::RenderTarget& rt, bool debug) {
	m_Env.Draw(rt);

	for (const auto& agent : m_Population) {
		agent.Draw(rt);

		if(debug)
			agent.DrawFitness(rt, m_Env);
	}
	
	rt.setView(rt.getDefaultView());

	DrawUI(rt);
}

void EvolutionTraining::DrawUI(sf::RenderTarget& rt) {
	Render::DrawStrings(rt, sf::Vector2i(0, 0), {
		"Alive: " + std::to_string(m_Population.size()),
		"Generation: " + std::to_string(m_Generation),
		"Iterations: " + std::to_string(m_IterationsNumber),
		"HighestGoal: " + std::to_string(m_HighestGoal),
		"HighestFitness: " + std::to_string(m_HighestFitness)
	});

	auto highest_goal = [](auto &l, auto &r){
		return l.CurrentGoal() < r.CurrentGoal();
	};
#if 0
	if(!m_Population.size())
		return;
	auto best = std::max_element(m_Population.begin(), m_Population.end(), highest_goal);

	Render::DrawStrings({ 0, 180 }, rt, {
		"BestGuy ",
		"GoalIndex " + std::to_string(best->CurrentGoal()),
		"Fitness " + std::to_string(best->FitnessFunction(m_Env))
	});
#endif
}

void EvolutionTraining::SaveBest(const std::string &filename)const{
	std::fstream best(filename, std::ios::binary | std::ios::out);

	if(!best.is_open()){
		Println("Can't open '%' to save BestOfEachGoal", filename);
		return;
	}

	Serializer<std::size_t>::ToStream(m_Generation, best);
	Serializer<std::size_t>::ToStream(m_IterationsNumber, best);
	Serializer<std::size_t>::ToStream(m_HighestGoal, best);
	Serializer<float>	   ::ToStream(m_HighestFitness, best);
	Serializer<std::vector<NeuralNetworkAgent>>::ToStream(m_BestOfEachGoal, best);

	Println("Saved % best Agents", m_BestOfEachGoal.size());
}

std::string EvolutionTraining::MakePath(size_t goal, size_t index) {
	return "best" + std::to_string(goal) + "/" + std::to_string(index) + ".bin";
}
