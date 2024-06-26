#pragma once

#include <cmath>
#include <optional>
#include <SFML/System/Vector2.hpp>
#include <SFML/Graphics/Transform.hpp>
#include "bsl/assert.hpp"

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

	inline sf::Vector2f Project(const sf::Vector2f& a, const sf::Vector2f& onto) {
		float dotAB = a.dot(onto);
		float dotBB = onto.dot(onto); // Equivalent to |b|^2, since |b|^2 = b � b

		if (dotBB == 0) {
			// Avoid division by zero if 'b' is a zero vector
			return {};
		}

		float projectionScalar = dotAB / dotBB;

		return projectionScalar * onto;
	}

	
	template<typename T>
	inline bool IsRectInside(const sf::Rect<T>& inner, const sf::Rect<T>& outer) {
		return (
			inner.left >= outer.left &&
			inner.top >= outer.top &&
			inner.left + inner.width <= outer.left + outer.width &&
			inner.top + inner.height <= outer.top + outer.height
		);
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

	inline std::optional<std::pair<float, sf::Vector2f>> RayLineIntersectionWithNormal(const sf::Vector2f& rayOrigin, const sf::Vector2f& rayDirection, const sf::Vector2f& lineStart, const sf::Vector2f& lineEnd) {
		auto v1 = rayOrigin - lineStart;
		auto v2 = lineEnd - lineStart;
		auto v3 = sf::Vector2f(-rayDirection.y, rayDirection.x);

		auto dot = v2.x * v3.x + v2.y * v3.y;
		if (std::abs(dot) < 0.000001)
			return {};

		auto t1 = (v2.x * v1.y - v2.y * v1.x) / dot;
		auto t2 = (v1.x * v3.x + v1.y * v3.y) / dot;
    
		if (t1 >= 0.0 && (t2 >= 0.0 && t2 <= 1.0)) {
			// Calculate the normal to the line
			auto normal = sf::Vector2f(lineEnd.y - lineStart.y, lineStart.x - lineEnd.x);
			float length = std::sqrt(normal.x * normal.x + normal.y * normal.y);
        
			// Normalize the normal if length is not zero
			if (length != 0.0f) {
				normal /= length;
			}

			normal *= -Math::Sign(normal.dot(rayDirection.normalized()));

			return std::make_pair(t1, normal);
		}
    
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

	inline float Clamp(float value, float min, float max) {
		if (value < min) return min;
		if (value > max) return max;
		return value;
	}

	inline float AngleCouterClockwize(const sf::Vector2f& v1, const sf::Vector2f& v2) {
		int angle = v1.normalized().angleTo(v2.normalized()).asDegrees();

		if(angle < 0)
			angle += 360;
		
		return angle % 360;
	}

	template<typename T>
	float AngleSigned(const sf::Vector2<T> &v1, const sf::Vector2<T> &v2){
		return sf::Vector2f(v1).normalized().angleTo(sf::Vector2f(v2).normalized()).asDegrees();
	}
	
	template<typename T>
	sf::Rect<T> MakeRect(sf::Vector2<T> first, sf::Vector2<T> second) {
		sf::Vector2<T> min = {
			std::min(first.x, second.x),
			std::min(first.y, second.y)
		};
		sf::Vector2<T> max = {
			std::max(first.x, second.x),
			std::max(first.y, second.y)
		};
		return {min, max - min};
	}

}//namespace Math::


static int &At(sf::Vector2i& vec, int axis){
	if (axis == 0)
		return vec.x;
	if (axis == 1)
		return vec.y;
	verify(false);
	return vec.x;
};

static const int &At(const sf::Vector2i& vec, int axis){
	if (axis == 0)
		return vec.x;
	if (axis == 1)
		return vec.y;
	verify(false);
	return vec.x;
};

struct AxisAlignedDirection2D{
	int Direction = 0;
	int Axis = 0;

	AxisAlignedDirection2D(int axis, int direction){
		Direction = direction;
		Axis = axis;

		verify(direction == 1 || direction == -1);
		verify(axis == 0 || axis == 1);
	}

	static std::optional<AxisAlignedDirection2D> Make(sf::Vector2i vector) {
		if (vector.x == 0 && std::abs(vector.y) == 1) {
			return AxisAlignedDirection2D(1, vector.y);
		}
		if (vector.y == 0 && std::abs(vector.x) == 1) {
			return AxisAlignedDirection2D(0, vector.x);
		}
		return std::nullopt;
	}
};

