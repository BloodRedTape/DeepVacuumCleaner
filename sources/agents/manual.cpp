#include "manual.hpp"
#include "config.hpp"

sf::Vector2f ManualAgent::Iterate(const VacuumCleaner& cleaner, const Environment& env)const {
	float forward = 0.f;	
	float rotate = 0.f;

	if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::W)) {
		forward += CleanerSpeed.x;
	}
	if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::S)) {
		forward -= CleanerSpeed.x;
	}

	if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::D)) {
		rotate += CleanerSpeed.y;
	}
	if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::A)) {
		rotate -= CleanerSpeed.y;
	}

	return {forward, rotate};
}
