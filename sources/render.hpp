#pragma once

#include <SFML/Graphics.hpp>

namespace Render{
    
    const sf::Font &GetFont();

	inline void DrawLine(sf::Vector2f start, sf::Vector2f end, float thichness, sf::RenderTarget &rt, sf::Color color = sf::Color::White) {
		auto direction = end - start;

		float WallLength = direction.length();

		if(std::abs(WallLength) < 0.01)
			return;

		sf::RectangleShape rect;
		rect.setPosition(start);
		rect.setSize({ WallLength, thichness});
		rect.setRotation(direction.angle());
		rect.setFillColor(color);
		rect.setOrigin({0, thichness/2.f});

		rt.draw(rect);
	}

	inline void DrawString(sf::Vector2f position, std::string str, sf::RenderTarget& rt) {
		sf::Text text(Render::GetFont(), str);
		text.setPosition(position);
		rt.draw(text);
	}

    inline void DrawCircle(sf::Vector2f position, float radius, sf::RenderTarget& rt, sf::Color color = sf::Color::White) {
		sf::CircleShape shape(radius);
		shape.setPosition((sf::Vector2f)position);
		shape.setOrigin({radius, radius});
		rt.draw(shape);
    }

	
inline sf::Color hsvToRgb(float h, float s, float v) {
    if (s <= 0.0f) {
        return sf::Color(static_cast<std::uint8_t>(v * 255), static_cast<std::uint8_t>(v * 255), static_cast<std::uint8_t>(v * 255));
    }

    float hh = h / 60.0f;
    int i = static_cast<int>(hh);
    float ff = hh - i;

    float p = v * (1.0f - s);
    float q = v * (1.0f - (s * ff));
    float t = v * (1.0f - (s * (1.0f - ff)));

    switch (i) {
    case 0:
        return sf::Color(static_cast<std::uint8_t>(v * 255), static_cast<std::uint8_t>(t * 255), static_cast<std::uint8_t>(p * 255));
    case 1:
        return sf::Color(static_cast<std::uint8_t>(q * 255), static_cast<std::uint8_t>(v * 255), static_cast<std::uint8_t>(p * 255));
    case 2:
        return sf::Color(static_cast<std::uint8_t>(p * 255), static_cast<std::uint8_t>(v * 255), static_cast<std::uint8_t>(t * 255));
    case 3:
        return sf::Color(static_cast<std::uint8_t>(p * 255), static_cast<std::uint8_t>(q * 255), static_cast<std::uint8_t>(v * 255));
    case 4:
        return sf::Color(static_cast<std::uint8_t>(t * 255), static_cast<std::uint8_t>(p * 255), static_cast<std::uint8_t>(v * 255));
    default:
        return sf::Color(static_cast<std::uint8_t>(v * 255), static_cast<std::uint8_t>(p * 255), static_cast<std::uint8_t>(q * 255));
    }
}

inline sf::Color GetRainbowColor(int index, int totalColors) {
	index = index % totalColors;
    const float hue = static_cast<float>(index) / static_cast<float>(totalColors) * 360.0f;
    return hsvToRgb(hue, 1.0f, 1.0f);
}


}