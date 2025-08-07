// Low-level vector math
#pragma once
#include <math.h>
#include <numbers>

namespace num = std::numbers;

namespace vec {
	const bool NEWLINE = true;
	const bool NO_NEWLINE = false;
	const bool AFTER_Y = true; // Unnecessary

	struct float3
	{
		union {
			struct { float x, y, z; };
			struct { float r, g, b; };
		};

		// Constructors
		// use parenthesis for implicit casts
		float3() : x(0.0f), y(0.0f), z(0.0f) {}
		float3(float x, float y, float z) : x(x), y(y), z(z) {}

		float3 operator*(float scalar) const {
			return float3{ x * scalar, y * scalar, z * scalar };
		}

		float3 operator/(float scalar) const {
			return float3{ x / scalar, y / scalar, z / scalar };
		}

		float3 operator+(float3 addend) const { // addend adds addend
			return float3{ x + addend.x, y + addend.y, z + addend.z };
		}

		float3 operator-(float3 subtrahend) const { // minuend subtracts subtrahend
			return float3{ x - subtrahend.x, y - subtrahend.y, z - subtrahend.z };
		}

		void operator+=(float3 addend) {
			x += addend.x;
			y += addend.y;
			z += addend.z;
		}

		// Calculates the dot product between this and another vector
		float dot(const float3 &other) const
		{
			return (x * other.x) + (y * other.y) + (z * other.z);
		}

		// TODO
		// Calculates the cross product between this and another vector
		float3 cross(const float3 &a, const float3 &other) const
		{

		}

		// TODO
		// Calculates vector rotated 90 degress clockwise around axis
		float3 rotateAround(float3 axis) const
		{

		}

		// Converts to RGBA32 value
		uint32_t toUint32() const
		{
			// Convert each color channel to 8 bit value
			uint8_t uint_r = r * 255;
			uint8_t uint_g = g * 255;
			uint8_t uint_b = b * 255;
			uint8_t uint_a = 255; // Full opacity

			// Pack each color channel byte into RGBA32 format (0xRRGGBBAA)
			return (uint_a << 24) | (uint_b << 16) | (uint_g << 8) | uint_r;
		}

		void print(const char *name = "") const {
			printf("%s:\tx: %f\ty: %f\tz: %f\n", name, x, y, z);
		}
	};

	struct float2
	{
		float x, y;

		// Constructors
		// use parenthesis for implicit casts
		float2() : x(0.0f), y(0.0f) {}
		float2(float x, float y) : x(x), y(y) {}
		float2(float3 input) : x(input.x), y(input.y) {}

		float2 operator*(float scalar) const {
			return float2{ x * scalar, y * scalar };
		}

		float2 operator/(float scalar) const {
			return float2{ x / scalar, y / scalar };
		}

		float2 operator+(float2 addend) const { // addend adds addend
			return float2{ x + addend.x, y + addend.y };
		}

		float2 operator-(float2 subtrahend) const { // minuend subtracts subtrahend
			return float2{ x - subtrahend.x, y - subtrahend.y };
		}

		void operator+=(float2 addend) {
			x += addend.x;
			y += addend.y;
		}

		// Gets vector rotated 90 degress clockwise
		float2 perpendicular() const
		{
			return float2{ y, -x };
		}

		// Calculates the dot product between this and another vector
		float dot(const float2 &other) const
		{
			return (x * other.x) + (y * other.y);
		}

		// TODO
		// Calculates cross product between this and another vector
		float2 cross( const float2 &other )
		{

		}

		// Calculates the length of float2 as a vector
		float length()
		{
			return std::sqrtf(x * x + y * y);
		}

		void print(const char *name = "") const {
			printf("%s:\tx: %f\ty: %f\n", name, x, y);
		}
	};

	// Very slow and straight-forward method for determining intersection of line and point (with radius)
	bool lineInterectsRadius( const float2 &vertA, const float2 &vertB, const float2 &point, const float radius = 0.708f )
	{
		float2 vecAB = vertB - vertA;
		float2 vecBA = vertA - vertB;
		float2 vecAP = point - vertA;
		float2 vecBP = point - vertB;
		float ABdotAP = vecAB.dot( vecAP );

		// If point is outside line segment, check instead if either end is within radius of point
		if ( ABdotAP < 0 ) {
			return (point - vertA).length() < radius;
		} else if ( vecBA.dot( vecBP ) < 0 ) {
			return (point - vertB).length() < radius;
		}

		// get angle between of AB and AP
		float distAB = vecAB.length();
		float distAP = vecAP.length();
		float angleBAP = std::acosf( ABdotAP / (distAB * distAP) );

		// find distance from point C to nearest point D along line AB
		float distDP = std::sinf( angleBAP ) * distAP;

		// return if that is within radius of point
		return distDP <= radius;
	}

	// Area of 2D triangle given three vertices (positive if clockwise, negative if counter-clockwise)
	float signedTriangleArea(const float2 &vertA, const float2 &vertB, const float2 &vertC) {
		float2 perpAB = (vertB - vertA).perpendicular();
		float2 vecAC = vertC - vertA;

		return perpAB.dot(vecAC) / 2;
	}

	// Normal-facing (visible) triangles' vertices are arranged clockwise by convention
	bool pointInsideTriangle(const float2 &vertA, const float2 &vertB, const float2 &vertC, const float2 &point, float3 &weights)
	{
		// Test if point is on right side of each edge segment
		float areaABP = signedTriangleArea(vertA, vertB, point);
		float areaBCP = signedTriangleArea(vertB, vertC, point);
		float areaCAP = signedTriangleArea(vertC, vertA, point);
		bool inTri = (areaABP >= 0) && (areaBCP >= 0) && (areaCAP >= 0);

		// Calculate weighting factors (barycentric coordinates)
		float totalArea = areaABP + areaBCP + areaCAP;
		float invTotalArea = 1 / totalArea;
		float weightA = areaABP * invTotalArea;
		float weightB = areaBCP * invTotalArea;
		float weightC = areaCAP * invTotalArea;
		weights = float3{ weightA, weightB, weightC };

		return inTri && totalArea > 0;
	}
}