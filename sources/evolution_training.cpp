#include "evolution_training.hpp"
#include "vacuum_cleaner_operator.hpp"
#include "render.hpp"
#include "random.hpp"
#include "math.hpp"

EvolutionTraining::EvolutionTraining(std::optional<std::string> filepath) {
	m_Env.LoadFromFile("test.map");

	if (filepath.has_value()) {

		for (int i = 0; ; i++) {
			std::string path = filepath.value() + "/" + std::to_string(i) + ".bin";
			if(!std::filesystem::exists(path))
				break;
			m_Population.push_back({});
			m_Population.front().Agent().LoadFromFile(path);
			m_Population.front().Iterate(m_Env, 0, 0);
		}
		
		m_Population = PopulationFromSorted(m_Population);
	}
	else {
		for(int i = 0; i<50; i++){
			m_Population.push_back({});
		}
	}
}

void EvolutionTraining::Tick(float dt) {
	m_IterationsNumber++;

	std::vector<int> die;
	
	for (int i = 0; i<m_Population.size(); i++) {
		auto &agent = m_Population[i];

		agent.Iterate(m_Env, dt, m_IterationsNumber);

		if(agent.StandStill() > StandStillToDie || agent.NumberFailure() || agent.HasCrashed(m_Env) || agent.Agent().HasEscaped())
			die.push_back(i);
	}

	if (die.size() > 2 && die.size() + 2 >= m_Population.size()) {
		die.pop_back();
		die.pop_back();
	}

	for (int i = die.size() - 1; i>=0; i--) {
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

	auto h = std::max_element(m_Population.begin(), m_Population.end(), [](auto &l, auto &r){
		return l.CurrentGoal() < r.CurrentGoal();
	})->CurrentGoal();

	if(h > m_HighestGoal){
		m_HighestGoal = h;
		SaveBest();
	}

	std::cout << "New Generation: " << m_Generation << "\n";

	std::sort(m_Population.begin(), m_Population.end(), [&](auto &left, auto &right) {
		return left.FitnessFunction(m_Env) > right.FitnessFunction(m_Env);
	});

	auto best_fitness = m_Population.front().FitnessFunction(m_Env);
	if (best_fitness > m_HighestFitness) {
		m_HighestFitness = best_fitness;
		SaveBest();
	}

	auto new_population = PopulationFromSorted(m_Population);

	m_Population = std::move(new_population);
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
		if (GetRandom<float>(0, 1) > 0.2)
			op = VacuumCleanerOperator::Mutate(op, Rate);
		op.Reset(m_StartingPosition);
	}
	
	for(int i = 0; i<count; i++){
		new_population.push_back(population[i]);
		new_population.back().Reset(m_StartingPosition);
	}
	
	return new_population;
}

void EvolutionTraining::Draw(sf::RenderTarget& rt) {
	m_Env.Draw(rt);

	for (const auto& agent : m_Population) {
		agent.Draw(rt);
	}
	
	rt.setView(rt.getDefaultView());

	DrawUI(rt);
}

void EvolutionTraining::DrawUI(sf::RenderTarget& rt) {
	Render::DrawString({0, 00}, "Alive: " + std::to_string(m_Population.size()), rt);
	Render::DrawString({0, 40}, "Generation: " + std::to_string(m_Generation), rt);
	Render::DrawString({0, 80}, "Iterations: " + std::to_string(m_IterationsNumber), rt);
	Render::DrawString({0, 120}, "HighestGoal: " + std::to_string(m_HighestGoal), rt);
}

void EvolutionTraining::SaveBest() {
	std::sort(m_Population.begin(), m_Population.end(), [&](auto &left, auto &right) {
		return left.FitnessFunction(m_Env) > right.FitnessFunction(m_Env);
	});

	for (int i = 0; i < std::min(ModelsToCrossover, m_Population.size()); i++) {
		m_Population.front().Agent().SaveToFile(MakePath(m_HighestGoal, i));
	}

	std::cout << "Saved\n";
}

std::string EvolutionTraining::MakePath(size_t goal, size_t index) {
	return "best" + std::to_string(goal) + "/" + std::to_string(index) + ".bin";
}
