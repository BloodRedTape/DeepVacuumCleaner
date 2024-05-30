#pragma once

#include <SFML/Graphics.hpp>

namespace Render{
    
    const sf::Font &GetFont();
    
    extern std::size_t s_DrawcallsCount;

    template<typename T>
	inline void DrawLine(sf::RenderTarget &rt, sf::Vector2<T> start, sf::Vector2<T> end, float thichness, sf::Color color = sf::Color::White) {
		auto direction = sf::Vector2f(end - start);

		float WallLength = direction.length();

		if(std::abs(WallLength) < 0.01)
			return;

		static sf::RectangleShape rect;
		rect.setPosition(sf::Vector2f(start));
		rect.setSize({ WallLength, thichness});
		rect.setRotation(direction.angle());
		rect.setFillColor(color);
		rect.setOrigin({0, thichness/2.f});

		rt.draw(rect);
        s_DrawcallsCount++;
	}

    template<typename T>
	inline void DrawString(sf::RenderTarget& rt, sf::Vector2<T> position, std::string str) {
		sf::Text text(Render::GetFont(), str);
		text.setPosition(sf::Vector2f(position));
		rt.draw(text);
        s_DrawcallsCount++;
	}

    template<typename T>
	inline void DrawStrings(sf::RenderTarget& rt, sf::Vector2<T> position, std::initializer_list<std::string> strings) {
        for (int i = 0; i < strings.size(); i++)
            DrawString(rt, sf::Vector2f(position) + sf::Vector2f(0, i * 40), strings.begin()[i]);
	}

    template<typename T>
    inline void DrawCircle(sf::RenderTarget& rt, sf::Vector2<T> position, float radius, sf::Color color = sf::Color::White, float outline = 0.f, sf::Color outline_color = sf::Color::White) {
		static sf::CircleShape shape;
        shape.setRadius(radius);
		shape.setPosition(sf::Vector2f(position));
		shape.setOrigin({radius, radius});
        shape.setFillColor(color);
        shape.setOutlineThickness(outline);
        shape.setOutlineColor(outline_color);
		rt.draw(shape);
        s_DrawcallsCount++;
    }
    
    template<typename T>
    inline void DrawRect(sf::RenderTarget& rt, sf::Vector2<T> position, sf::Vector2<T> size, sf::Color color = sf::Color::White, float outline = 0.f, sf::Color outline_color = sf::Color::White) {
        static sf::RectangleShape rect;
        rect.setPosition(sf::Vector2f(position));
        rect.setSize(sf::Vector2f(size));
        rect.setFillColor(color);
        rect.setOutlineThickness(outline);
        rect.setOutlineColor(outline_color);
        rt.draw(rect);
        s_DrawcallsCount++;
    }

    template<typename T>
    inline void DrawRect(sf::RenderTarget& rt, sf::Rect<T> rect_size, sf::Color color = sf::Color::White, float outline = 0.f, sf::Color outline_color = sf::Color::White) {
        static sf::RectangleShape rect;
        rect.setPosition(sf::Vector2f(rect_size.getPosition()));
        rect.setSize(sf::Vector2f(rect_size.getSize()));
        rect.setFillColor(color);
        rect.setOutlineThickness(outline);
        rect.setOutlineColor(outline_color);
        rt.draw(rect);
        s_DrawcallsCount++;
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