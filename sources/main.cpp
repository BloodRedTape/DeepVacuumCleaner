#include "matrix.hpp"
#include "nn.hpp"
#include "random.hpp"
#include "render.hpp"
#include "math.hpp"
#include "application.hpp"
#include "vacuum_cleaner.hpp"
#include "map_editor.hpp"
#include "evolution_training.hpp"
#include <iostream>
#include <sstream>


class EvolutionTrainingApp: public ZoomMoveApplication{
	using Super = ZoomMoveApplication;
private:
	EvolutionTraining m_Evo;

	bool m_IsPaused = false;
	bool m_IsDebug = false;
public:
	EvolutionTrainingApp(sf::Vector2i size, std::optional<std::string> filepath):
		Super(size),
		m_Evo(filepath)
	{
		m_View.zoom(2);
		m_Window.setFramerateLimit(1000);
	}

	virtual void Tick(float dt) override{
		Super::Tick(dt);

		if(m_IsPaused)
			return;

		int num_per_frame = 150;
		for (int i = 0; i < num_per_frame; i++) {
			m_Evo.Tick(dt / num_per_frame);
		}
	}

	virtual void OnEvent(const sf::Event& e) override{ 
		Super::OnEvent(e);

		if(e.type == sf::Event::KeyPressed && e.key.code == sf::Keyboard::Key::S && sf::Keyboard::isKeyPressed(sf::Keyboard::Key::LControl)){
			m_Evo.SaveBest();
			std::cout << "Saved\n";
		}

		if (e.type == sf::Event::KeyPressed && e.key.code == sf::Keyboard::Key::Space) {
			m_IsPaused = !m_IsPaused;
		}

		if (e.type == sf::Event::KeyPressed && e.key.code == sf::Keyboard::Key::D) {
			m_IsDebug = !m_IsDebug;
		}
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
		m_Cleaner.Agent().LoadFromFile("best6/0.bin");
		m_Env.LoadFromFile("race.map");
		m_Window.setFramerateLimit(120);
		m_View.zoom(3);
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
	return std::make_unique<EvolutionTrainingApp>(size, std::optional<std::string>{"best4"});
}

int main()
{
	srand(time(0));
	
	MakeApp<MapEditor>({1720, 1080})->Run();
	
	return 0;
}
