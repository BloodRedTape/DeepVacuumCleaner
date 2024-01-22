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

		int num_per_frame = 100;
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
	}

	virtual void Render(sf::RenderTarget& rt) override{
		Super::Render(rt);

		m_Evo.Draw(rt);
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
		m_Cleaner.Agent().LoadFromFile("best8/0.bin");
		m_Env.LoadFromFile("test.map");
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

int main()
{
	srand(time(0));
	Render::Font.loadFromFile(R"(C:\Users\E1\Downloads\BRLNSR.ttf)");
	Render::Font.setSmooth(true);

	EvolutionTrainingApp app({1919, 1080}, {"best7"});
	//EvolutionDemoApp app({1920, 1080});
	//MapEditor app({1920, 1080});
	app.Run();
	
	return 0;
}
