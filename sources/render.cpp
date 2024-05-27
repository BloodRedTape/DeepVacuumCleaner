#include "render.hpp"

namespace Render {
	static sf::Font MakeFont(const char *path) {
		auto font = sf::Font::loadFromFile(path);
		assert(font.has_value());

		font.value().setSmooth(true);
		return std::move(font.value());
	}
	
	const sf::Font &GetFont(){
		static sf::Font font = MakeFont(R"(X:\User\Fonts\Montserrat\Montserrat-Medium.ttf)");
		return font;
	}
}