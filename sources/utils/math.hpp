#pragma once

#include <cmath>
#include <optional>
#include <SFML/System/Vector2.hpp>
#include <SFML/Graphics/Transform.hpp>


namespace Math{

	inline float Sigmoid(float x) {
		return 1.f / (1.f + expf(-x));
	}

	inline float None(float x) {
		return x;
	}
	
	template<typename T>
	T Sign(T value) {
		return value >= 0 ? 1 : -1;
	}

	constexpr sf::Vector2f Forward{1.f, 0};

	inline sf::Vector2f RotationToDirection(float rotation){
		sf::Transform t;
		t.rotate(sf::degrees(rotation));
		return t * Forward;
	}


	inline std::optional<float> RayLineIntersection(const sf::Vector2f& rayOrigin, const sf::Vector2f& rayDirection, const sf::Vector2f& lineStart, const sf::Vector2f& lineEnd) {
		auto v1 = rayOrigin - lineStart;
        auto v2 = lineEnd - lineStart;
        auto v3 = sf::Vector2f(-rayDirection.y, rayDirection.x);


        auto dot = v2.dot(v3);
        if (std::abs(dot) < 0.000001)
            return {};

        auto t1 = v2.cross(v1) / dot;
        auto t2 = v1.dot(v3) / dot;

        if (t1 >= 0.0 && (t2 >= 0.0 && t2 <= 1.0))
            return t1;

        return {};
	}

	inline bool LineCircleIntersection(sf::Vector2f start, sf::Vector2f end, sf::Vector2f circle, float radius) {
		// Calculate coefficients for the quadratic equation
		float dx = end.x - start.x;
		float dy = end.y - start.y;

		float a = dx * dx + dy * dy;
		float b = 2 * (dx * (start.x - circle.x) + dy * (start.y - circle.y));
		float c = (start.x - circle.x) * (start.x - circle.x) + (start.y - circle.y) * (start.y - circle.y) - radius * radius;

		// Calculate the discriminant
		float discriminant = b * b - 4 * a * c;

		if (discriminant < 0) {
			// No intersection
			return false;
		}

		// Calculate the two possible values for t
		float t1 = (-b - sqrt(discriminant)) / (2 * a);
		float t2 = (-b + sqrt(discriminant)) / (2 * a);

		// Check if the intersection points are within the line segment
		if (t1 >= 0 && t1 <= 1) {
			return true;
		}

		if (t2 >= 0 && t2 <= 1) {
			return true;
		}

		// No intersection within the line segment
		return false;
	}

	inline bool LineIntersect(sf::Vector2f p1, sf::Vector2f q1, sf::Vector2f p2, sf::Vector2f q2) {
		auto orientation = [](sf::Vector2f p, sf::Vector2f q, sf::Vector2f r) {
			float val = (q.y - p.y) * (r.x - q.x) - (q.x - p.x) * (r.y - q.y);
			if (val == 0) return 0;  // collinear
			return (val > 0) ? 1 : 2; // clock or counterclock wise
		};

	   int o1 = orientation(p1, q1, p2);
	   int o2 = orientation(p1, q1, q2);
		int o3 = orientation(p2, q2, p1);
		int o4 = orientation(p2, q2, q1);

		if (o1 != o2 && o3 != o4) return true;  // General case

		return false;  // Doesn't handle special cases (collinear points)
	}

	inline bool LineRectIntersection(sf::Vector2f start, sf::Vector2f end, sf::FloatRect rect) {
		// Get the four corners of the rectangle
		sf::Vector2f topLeft(rect.left, rect.top);
		sf::Vector2f topRight(rect.left + rect.width, rect.top);
		sf::Vector2f bottomLeft(rect.left, rect.top + rect.height);
		sf::Vector2f bottomRight(rect.left + rect.width, rect.top + rect.height);

		// Check if the line intersects any of the four sides of the rectangle
		if (LineIntersect(start, end, topLeft, topRight) ||
			LineIntersect(start, end, topRight, bottomRight) ||
			LineIntersect(start, end, bottomRight, bottomLeft) ||
			LineIntersect(start, end, bottomLeft, topLeft)) {
			return true;
		}

		return false;
	}

	inline bool IsBad(float num) {
		return std::isinf(num) || std::isnan(num);
	}

}//namespace Math::
