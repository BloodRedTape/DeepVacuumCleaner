#pragma once

#include <optional>
#include <SFML/System/Vector2.hpp>
#include "environment.hpp"
#include "vacuum_cleaner.hpp"
#include "application.hpp"

class MapEditor: public ZoomMoveApplication{
	using Super = ZoomMoveApplication;
private:
	std::optional<sf::Vector2i> m_WallBegin;

	Environment m_Env;
	VacuumCleaner m_Cleaner;

	bool m_DrawBounds = true;
	bool m_DrawGridDecomposition = true;
	bool m_DrawNumbers = false;

	sf::Vector2i m_GridCellSize = {20, 20};
	sf::Vector2i m_StartPosition = {50, 50};
public:

	MapEditor(sf::Vector2i world_size);

	void Tick(float dt)override;

	void OnImGui()override;

	void Render(sf::RenderTarget& rt)override;

	void OnEvent(const sf::Event& e)override;

	void OnSave();
};