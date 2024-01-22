#include "render.hpp"

namespace Render {
	static sf::Font MakeFont(const char *path) {
		sf::Font font;
		font.loadFromFile(path);
		font.setSmooth(true);
		return font;
	}
	
	const sf::Font &GetFont(){
		static sf::Font font = MakeFont(R"(C:\Users\E1\Downloads\BRLNSR.ttf)");
		return font;
	}
}