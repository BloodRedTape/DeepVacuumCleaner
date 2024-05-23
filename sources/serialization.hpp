#pragma once

#include <iostream>
#include <vector>
#include <string>
#include <optional>
#include <SFML/System/Vector2.hpp>

template<typename T>
struct RawSerializer {
	static_assert(!std::is_pointer_v<T>, "Can't raw-serialize pointers");

	static void ToStream(const T& object, std::ostream& stream) {
		stream.write((const char*)std::addressof(object), sizeof(object));
	}

	static std::optional<T> FromStream(std::istream& stream) {
		T object;

		if (!stream.read((char*)std::addressof(object), sizeof(object)))
			return std::nullopt;

		return { std::move(object) };
	}
};

template<typename T>
struct Serializer {
	static void ToStream(const T& object, std::ostream& stream) { 
		RawSerializer<T>::ToStream(object, stream); 
	}

	static std::optional<T> FromStream(std::istream& stream) { 
		return RawSerializer<T>::FromStream(stream);
	}
};

template<>
struct Serializer<std::string>{
	static void ToStream(const std::string& string, std::ostream& stream) {
		Serializer<std::size_t>::ToStream(string.size(), stream);
		stream.write(string.data(), string.size());
	}

	static std::optional<std::string> FromStream(std::istream& stream) {
		auto size = Serializer<std::size_t>::FromStream(stream);

		if(!size.has_value())
			return std::nullopt;

		std::string string(size.value(), '\0');

		if(!stream.read(string.data(), string.size()))
			return std::nullopt;

		return {std::move(string)};
	}
};

template<typename T>
struct Serializer<std::vector<T>>{

	static void ToStream(const std::vector<T>& vector, std::ostream& stream) {
		Serializer<std::size_t>::ToStream(vector.size(), stream);

		for (const auto &e : vector)
			Serializer<T>::ToStream(e, stream);
	}

	static std::optional<std::vector<T>> FromStream(std::istream& stream) {
		auto size = Serializer<std::size_t>::FromStream(stream);

		if(!size.has_value())
			return std::nullopt;

		std::vector<T> vector;
		
		for(int i = 0; i<size.value(); i++) {
			auto e = Serializer<T>::FromStream(stream);

			if(!e.has_value())
				return std::nullopt;

			vector.emplace_back(std::move(e.value()));
		}

		return {std::move(vector)};
	}

};
