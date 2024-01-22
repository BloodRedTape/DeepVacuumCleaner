#pragma once

#include <iostream>
#include <vector>
#include <string>
#include <SFML/System/Vector2.hpp>

namespace Serialization{
	
	inline void ToStream(float e, std::ostream& stream) {
		stream.write((char*)&e, sizeof(e));
	}

	template<typename T>
	inline T FromStream(std::istream& stream) {
		return FromStreamImpl(stream, (T*)nullptr);
	}

	inline float FromStreamImpl(std::istream& stream, float*) {
		float f;
		stream.read((char*)&f, sizeof(f));
		return f;
	}

	inline void ToStream(int e, std::ostream& stream) {
		stream.write((char*)&e, sizeof(e));
	}

	inline int FromStreamImpl(std::istream& stream, int*) {
		int f;
		stream.read((char*)&f, sizeof(f));
		return f;
	}

	inline void ToStream(const std::string &string, std::ostream& stream) {
		size_t s = string.size();
		stream.write((char*)&s, sizeof(s));
		stream.write(string.data(), string.size());
	}

	inline std::string FromStreamImpl(std::istream& stream, std::string*) {
		size_t s;

		stream.read((char*)&s, sizeof(s));
		std::string string(s, '\0');
		stream.read(string.data(), string.size());
		return string;
	}
	
	template<typename T>
	inline void ToStream(const std::vector<T>& vector, std::ostream& stream) {
		size_t size = vector.size();
		stream.write((char*)&size, sizeof(size));

		for (const auto &e : vector) {
			ToStream(e, stream);
		}
	}

	template<typename T>
	inline std::vector<T> FromStreamImpl(std::istream& stream, std::vector<T>*) {
		size_t size;
		stream.read((char*)&size, sizeof(size));

		std::vector<T> vector;
		
		for(int i = 0; i<size; i++){
			auto e = FromStream<T>(stream);
			vector.emplace_back(std::move(e));
		}

		return vector;
	}

	inline void ToStream(sf::Vector2i vector, std::ostream& stream) {
		stream.write((char*)&vector, sizeof(vector));
	}

	inline sf::Vector2i FromStreamImpl(std::istream& stream, sf::Vector2i*) {
		sf::Vector2i f;
		stream.read((char*)&f, sizeof(f));
		return f;
	}
}
