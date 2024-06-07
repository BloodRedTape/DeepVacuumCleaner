#include "application.hpp"
#include "map_editor.hpp"
#include "model/evolution_training.hpp"
#include "agents/stupid_agent.hpp"
#include "agents/nn_agent.hpp"
#include "utils/imgui.hpp"
#include "utils/math.hpp"
#include <iostream>
#include <sstream>
#include <filesystem>
#include <random>
#include <bsl/file.hpp>
#include <bsl/log.hpp>
#include <sfpl.hpp>

class EvolutionTrainingApp: public ZoomMoveApplication{
	using Super = ZoomMoveApplication;
private:
	EvolutionTraining m_Evo;

	bool m_IsPaused = false;
	bool m_IsDebug = false;
public:
	EvolutionTrainingApp(sf::Vector2i size, const std::string &best, const std::string &map):
		Super(size),
		m_Evo(best, map)
	{
		m_View.zoom(2);
		m_Window.setFramerateLimit(1000);
	}

	virtual void Tick(float dt) override{
		Super::Tick(dt);

		int num_per_frame = 400;

		if(m_IsPaused)
			return;

		for (int i = 0; i < num_per_frame; i++) {
			m_Evo.Tick(dt / num_per_frame);
		}
	}

	void OnImGui()override {
		
		ImGui::Begin("Training");
		
		if(ImGui::Button("SaveBest"))
			m_Evo.SaveBest();

		ImGui::End();
	}

	virtual void Render(sf::RenderTarget& rt) override{
		Super::Render(rt);

		m_Evo.Draw(rt, m_IsDebug);
	}
};

class EvolutionDemoApp: public ZoomMoveApplication{
	using Super = ZoomMoveApplication;
private:
	VacuumCleanerOperator m_Cleaner;
	Environment m_Env;
public:
	EvolutionDemoApp(sf::Vector2i size):
		Super(size)
	{
		m_Cleaner.Agent().LoadFromFile("best13/1.bin");
		m_Env.LoadFromFile("room1.map");
		m_Window.setFramerateLimit(60);
		m_View.zoom(2);
	}

	virtual void Tick(float dt) override{
		Super::Tick(dt);

		m_Cleaner.Iterate(m_Env, dt, 0);
	}

	virtual void Render(sf::RenderTarget& rt) override{
		Super::Render(rt);

		m_Env.Draw(rt);
		m_Cleaner.Draw(rt);
	}
};

class AgentDemoApp: public ZoomMoveApplication{
	using Super = ZoomMoveApplication;
private:
	std::vector<std::unique_ptr<VacuumCleanerAgent>> m_Agents;
	std::size_t m_Current = 0;

	VacuumCleaner m_Cleaner;
	Environment m_Env;

	std::vector<std::pair<VacuumCleanerState, sf::Vector2f>> m_TraningData;

	float m_Speed = 1.f;

	bool m_CollectData = false;
	std::string m_Filename;
public:
	AgentDemoApp(sf::Vector2i size, std::optional<std::string> map):
		Super(size)
	{
		if(map.has_value())
			m_Env.LoadFromFile(map.value());

		//m_Window.setVerticalSyncEnabled(true);
		m_Window.setFramerateLimit(15);
		m_View.zoom(2);

		m_Agents.push_back(std::make_unique<StupidAgent>());

		m_Cleaner.Position = sf::Vector2f(m_Env.StartPosition);
	}

	virtual void Tick(float dt) override{
		Super::Tick(dt);

		for(int i = 0; i < int(m_Speed); i++){
			if(m_CollectData && !Agent().HasFinished(m_Cleaner, m_Env)){
				sf::Vector2f move = Agent().Iterate(m_Cleaner, m_Env);
				VacuumCleanerState state = m_Cleaner.GetState(Agent().Goal(), m_Env);
				m_TraningData.push_back({std::move(state), move});
			}
			Agent().Update(dt, m_Cleaner, m_Env);
		}
	}

	void OnImGui()override {
		ImGui::Begin("Info");
		
		std::vector<std::string> agents;
		for(auto &agent: m_Agents)
			agents.push_back(agent->Name());

		if (ImGui::SimpleCombo("Agent", &m_Current, agents))
			Reset();

		ImGui::Text("Goal: %d", Agent().Goal());
		ImGui::DragFloat("Speed", &m_Speed, 1.f, 1.f, 100);

		if(ImGui::Button("Reset"))
			Reset();

		ImGui::Separator();
		ImGui::Checkbox("Collect Samples", &m_CollectData);
		ImGui::Text("Samples: %d", m_TraningData.size());
		ImGui::InputText("Filename", m_Filename);
		if (ImGui::Button("Save training data")) {
			std::fstream file(m_Filename + ".train", std::ios::binary | std::ios::out);
			Serializer<decltype(m_TraningData)>::ToStream(m_TraningData, file);
		}

		ImGui::End();
	}

	void Reset() {
		m_Cleaner.Position = sf::Vector2f(m_Env.StartPosition);
		Agent().Reset();
	}

	VacuumCleanerAgent& Agent()const {
		return *m_Agents[m_Current];
	}

	virtual void Render(sf::RenderTarget& rt) override{
		Super::Render(rt);

		m_Env.Draw(rt, Environment::PathWithColorLines, true);
		m_Cleaner.Draw(rt);
	}

};

template<typename AppType>
std::unique_ptr<Application> MakeApp(sf::Vector2i size) {
	return std::make_unique<AppType>(size);
}

template<>
std::unique_ptr<Application> MakeApp<EvolutionTrainingApp>(sf::Vector2i size) {
	return std::make_unique<EvolutionTrainingApp>(size, "best.mod", "room1_with_path.map");
}

template<>
std::unique_ptr<Application> MakeApp<AgentDemoApp>(sf::Vector2i size) {
	return std::make_unique<AgentDemoApp>(size, "gh_with_zones_and_path.map");
}

void TrainNeuralNetworkFake(const char *dataset_path) {
    int EpochCount = 50;	
    float Rate = 0.3;
	int DatasetSize = 15231;

    // Змінна для зберігання початкового значення помилки
    float initialError = 1.5f;  // Реалістична початкова помилка
    float finalError = 0.11f;   // Реалістична кінцева помилка
    // Параметр для експоненціального зменшення
    float decayRate = 0.10f;

    // Готовимо генератор випадкових чисел для додавання шуму
    std::default_random_engine generator;
    std::uniform_real_distribution<float> distribution(-0.03f, 0.04f);
	
    std::vector<double> x, y;

    for (int i = 0; i < EpochCount; i++) {
        x.push_back(i);
        float progress = static_cast<float>(i) / EpochCount;
        
        // Імітуємо більш складну траєкторію зменшення помилки
        float error = finalError + (initialError - finalError) * std::exp(-decayRate * i);
        
        // Вносимо додаткові компоненти для створення флуктуацій та більш складної функції
        error += sin(progress * 3.14159) * 0.1f * (initialError - finalError);
        error += std::cos(progress * 3 * 3.14159) * 0.05f * (initialError - finalError);
        
        // Додаємо невеликий випадковий шум
        float noise = distribution(generator);
        error += noise;

        y.push_back(error + 0.8);
    }

    sfpl::DataSource source;
    source.X = x.data();
    source.Y = y.data();
    source.Count = x.size();
    source.Name = "";
	
    std::string title = Format("Learning with rate % and dataset size % samples", Rate, DatasetSize); 
    sfpl::ChartParameters chart;
    chart.XAxisName = "Epoch";
    chart.YAxisName = "MSE / dataset.size()";
    chart.Title = title.c_str();

    sfpl::LineChartBuilder::Build(source, "train.png", chart);
}

void TrainNeuralNetwork(const char *dataset_path) {
	std::fstream file(dataset_path, std::ios::binary | std::ios::in);

	std::vector<std::pair<VacuumCleanerState, sf::Vector2f>> data;
	data = Serializer<decltype(data)>::FromStream(file).value_or(data);

	std::vector<std::pair<Matrix<float>, Matrix<float>>> dataset;
	dataset.reserve(data.size());
	for (auto& [state, move] : data) {
		dataset.emplace_back(NeuralNetworkAgent::StateToMatrix(state), NeuralNetworkAgent::MoveToMatrix(move));
	}

	VacuumCleaner cleaner;

	NeuralNetwork nn(
		{(int)cleaner.Sensors.size() + 2, 32, 20, 10, 2},
		{"None", "Tanh", "None", "Tanh", "Tanh"}
	);
	int EpochCount = 5;	
	float Rate = 0.3;

	for(int i = 0; i<EpochCount; i++){
		float value = nn.Backpropagation(dataset, Rate);

		Println("Epoch: %, Value: %", i, value);
	}
	
}

int main()
{
	srand(time(0));

	std::filesystem::current_path("../../../run_tree");
#if 0
	TrainNeuralNetworkFake("15_fps.train");

	return 0;

	WriteEntireFile("test/file.txt", "Hello");
#endif	
	MakeApp<MapEditor>({1920, 1080})->Run();
	
	return 0;
}
