#include "nn_agent.hpp"
#include "utils/math.hpp"
#include "utils/render.hpp"

NNAgent::NNAgent(NeuralNetwork &&nn):
	m_NN(std::move(nn))
{}

sf::Vector2f NNAgent::Iterate(const VacuumCleaner &cleaner, const Environment & env)const {
	sf::Vector2f goal(env.Path[m_CurrentGoal]);

	auto direction = goal - cleaner.Position;
	
	sf::Vector2f trench = cleaner.Position + direction.normalized() * CleanerSpeed.x;

	m_Trench = trench;

	
	auto state = cleaner.GetState(m_CurrentGoal, env);

	for (int i = 0; i < cleaner.Sensors.size(); i++) {
#if 0
		sf::Vector2f sensor_dir = cleaner.Direction().rotatedBy(sf::degrees(cleaner.Sensors[i].Rotation)).normalized();
		sf::Vector2f sensor_vector = sensor_dir * state.SensorsData[i];
		
		sf::Vector2f trench_on_sensor = Math::Project(trench, sensor_vector);

		if (trench_on_sensor.length() > CleanerRadius)
			continue;
		
		float overlap = CleanerRadius - trench_on_sensor.length();

		trench += state.SensorsIntersectionNormal[i].normalized() * overlap;
#else
		if(state.IsCollided(i))
			trench += state.SensorsIntersectionNormal[i] * (state.SensorsData[i]);
#endif
	}

	m_OffsetedTrench = trench;
	
	auto angle = Math::AngleSigned(cleaner.Direction(), (trench - cleaner.Position).normalized());
	float forward = CleanerSpeed.x;
	float rotation = Math::Sign(angle) * CleanerSpeed.y;

	if (std::abs(angle) > 10)
		return {forward * ((angle/180.f)), rotation};
	
	return {forward, rotation};
}

void NNAgent::DrawDebugData(sf::RenderTarget& rt) {
	Render::DrawCircle(rt, m_Trench, 5.f, sf::Color::Green);
	Render::DrawCircle(rt, m_OffsetedTrench, 5.f, sf::Color::Blue);
}
