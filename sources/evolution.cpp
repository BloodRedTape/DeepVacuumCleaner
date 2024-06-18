#include "application.hpp"
#include "map_editor.hpp"
#include "model/evolution_training.hpp"
#include "agents/stupid_agent.hpp"
#include "agents/nn_agent.hpp"
#include "agents/manual.hpp"
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

int main(){
	srand(time(0));

	std::filesystem::current_path("../../../run_tree");

	EvolutionTrainingApp({1920, 1080}, "best.mod", "room1_with_path.map").Run();
}
