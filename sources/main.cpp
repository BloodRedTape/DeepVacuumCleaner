#include "application.hpp"
#include "map_editor.hpp"
#include "model/evolution_training.hpp"
#include "utils/imgui.hpp"
#include "utils/math.hpp"
#include <iostream>
#include <sstream>
#include <filesystem>
#include <bsl/file.hpp>
#include <bsl/log.hpp>

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

template<typename AppType>
std::unique_ptr<Application> MakeApp(sf::Vector2i size) {
	return std::make_unique<AppType>(size);
}

template<>
std::unique_ptr<Application> MakeApp<EvolutionTrainingApp>(sf::Vector2i size) {
	return std::make_unique<EvolutionTrainingApp>(size, "best.mod", "room1_with_path.map");
}

int main()
{
	srand(time(0));

	std::filesystem::current_path("../../../run_tree");

	WriteEntireFile("test/file.txt", "Hello");
	
	MakeApp<MapEditor>({1920, 1080})->Run();
	
	return 0;
}
