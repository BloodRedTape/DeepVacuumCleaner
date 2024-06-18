#include <optional>
#include "application.hpp"
#include "agents/stupid_agent.hpp"
#include "agents/nn_agent.hpp"
#include "agents/manual.hpp"
#include "utils/imgui.hpp"
#include "utils/math.hpp"

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
	
	bool m_DrawSensorsState = false;

	bool m_DrawPath = false;
	bool m_DrawZones = false;
	bool m_DrawCoveredPath = true;
	bool m_CollectCoveredPath = true;

	std::vector<sf::CircleShape> m_CoveredPath;
	float m_CoveredPathSampleRate = 0.3f;
	float m_Timer = 0;
	std::size_t m_CollisionsCount = 5;
public:
	AgentDemoApp(sf::Vector2i size, std::optional<std::string> map):
		Super(size)
	{
		if(map.has_value())
			m_Env.LoadFromFile(map.value());

		//m_Window.setVerticalSyncEnabled(true);
		m_Window.setFramerateLimit(60);
		m_View.zoom(2);

		m_Agents.push_back(std::make_unique<NNAgent>());
		m_Agents.push_back(std::make_unique<StupidAgent>());
		m_Agents.push_back(std::make_unique<ManualAgent>());

		Reset();
	}

	virtual void Tick(float dt) override{
		Super::Tick(dt);


		for(int i = 0; i < int(m_Speed); i++){
			if(m_CollectData && !Agent().HasFinished(m_Cleaner, m_Env)){
				sf::Vector2f move = Agent().Iterate(m_Cleaner, m_Env);
				VacuumCleanerState state = m_Cleaner.GetState(Agent().Goal(), m_Env);
				m_TraningData.push_back({std::move(state), move});
			}

			m_Timer += dt;

			Agent().Update(dt, m_Cleaner, m_Env);
			
			if(m_Timer > m_CoveredPathSampleRate){
				m_Timer -= m_CoveredPathSampleRate;

				if(m_CollectCoveredPath){

					sf::CircleShape circle(CleanerRadius);
					//circle.setFillColor(sf::Color(255, 255, 255, 60));
					circle.setFillColor(sf::Color(80, 80, 80, 255));
					circle.setPosition(m_Cleaner.Position);
					circle.setOrigin({CleanerRadius, CleanerRadius});
					m_CoveredPath.push_back(circle);
				}
			}
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
		ImGui::Text("Walls Colisions: %d", m_CollisionsCount);
		ImGui::DragFloat("Speed", &m_Speed, 1.f, 1.f, 100);

		if(ImGui::Button("Reset"))
			Reset();

		ImGui::Separator();

		ImGui::Checkbox("Collect Covered Space", &m_CollectCoveredPath);
		ImGui::Checkbox("Draw Covered Space", &m_DrawCoveredPath);
		ImGui::InputFloat("Covered Space Sample Rate (seconds)", &m_CoveredPathSampleRate);
		ImGui::Separator();

		ImGui::Checkbox("Draw Sensors State", &m_DrawSensorsState);
		ImGui::Separator();
		ImGui::Checkbox("Collect Samples", &m_CollectData);
		ImGui::Text("Samples: %d", m_TraningData.size());
		ImGui::InputText("Filename", m_Filename);
		if (ImGui::Button("Save training data")) {
			std::fstream file(m_Filename + ".train", std::ios::binary | std::ios::out);
			Serializer<decltype(m_TraningData)>::ToStream(m_TraningData, file);
		}
		ImGui::Separator();
		for (auto file : std::filesystem::directory_iterator(".")) {
			if(!file.is_regular_file())
				continue;

			if(file.path().extension() != ".map")
				continue;

			if (ImGui::Button(file.path().string().c_str())) {
				m_Env.Clear();
				m_Env.LoadFromFile(file.path().string());
			}
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

		m_Env.Draw(rt, m_DrawPath ? Environment::PathWithColorLines : Environment::NoPath, m_DrawZones);

		for(const auto &shape: m_CoveredPath){
			rt.draw(shape);
		}

		m_Cleaner.Draw(rt);

		if(m_DrawSensorsState)
			m_Cleaner.DrawIntersections(rt, m_Env);

		Agent().DrawDebugData(rt);
	}

};


int main() {
	std::filesystem::current_path("../../../run_tree");

	AgentDemoApp({1920, 1080}, "diplom_demo.map").Run();
}
