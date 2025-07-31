#pragma once
#include <math.h>
#include "float_debug.h"

namespace vec {
	const bool NEWLINE = true;
	const bool NO_NEWLINE = false;
	const bool AFTER_Y = true;

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

		void print(const char *name = nullptr, const bool addNewline = true, const bool afterY = true) const {
			if (name) {
				std::cout << name << "\t";
			}
			else {
				std::cout << "\t\t";
			}
			std::cout << x << '\t' << y << '\t' << z;
			if (addNewline) {
				std::cout << '\n';
			}
		}
	};

	struct float2
	{
		float x, y;

		// Constructors
		// use parenthesis for implicit casts
		float2() : x(0.0f), y(0.0f) {}
		float2(float x, float y) : x(x), y(y) {}

		float2 operator/(float scalar) const {
			return float2{ x / scalar, y / scalar };
		}

		float2 operator*(float scalar) const {
			return float2{ x * scalar, y * scalar };
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

		// Calculates the length of a vector
		float length()
		{
			return std::sqrtf(x * x + y * y);
		}

		void print(const char *name = nullptr, const bool addNewline = true, const bool afterY = true) const {
			if (name) {
				printFloatY(y, name, NO_NEWLINE);
			}
			else {
				printFloatY(y, NO_NEWLINE);
			}
			printFloatX(x, addNewline, afterY);
		}
	};


	// Creates vector with origin a and destination b
	float2 vec2d(float2 a, float2 b)
	{
		return float2{ b.x - a.x, b.y - a.y };
	}

	// Calculates the dot product between two vectors
	float dot(float2 a, float2 b)
	{
		return a.x * b.x + a.y * b.y;
	}

	// Rotates vector 90 degress clockwise
	float2 perpendicular(float2 vec)
	{
		return float2{ vec.y, -vec.x };
	}

	bool pointInsideRadius(float2 a, float2 p, const float radius = 0.708f, const bool verbose = false) {
		float2 vecAP = vec2d(a, p);
		float distAP = vecAP.length();
		
		if (verbose) {
			std::cout << "dist\t" << distAP << "\nHit:" << (distAP < radius) << std::endl;
		}

		return distAP < radius;
	}

	// Very slow and straight-forward method for determining intersection of line and point (with radius)
	bool lineInterectsRadius(const float2 &a, const float2 &b, const float2 &p, const float radius = 0.708f, const bool verbose = false) {
		float2 vecAB = vec2d(a, b);
		float2 vecBA = vec2d(b, a);
		float2 vecAP = vec2d(a, p);
		float2 vecBP = vec2d(b, p);
		float ABdotAP = dot(vecAB, vecAP);
		float BAdotBP = dot(vecBA, vecBP);

		// If point is outside line segment, check point radii
		if ((ABdotAP < 0) || (BAdotBP < 0)) {
			return pointInsideRadius(a, p, radius) || pointInsideRadius(b, p, radius);
		}

		// get angle between of AB and AP
		float distAB = vecAB.length();
		float distAP = vecAP.length();
		float angleBAP = std::acosf(ABdotAP / (distAB * distAP));

		// find distance from point C to nearest point D along line AB
		float distDP = std::sinf(angleBAP) * distAP;

		if (verbose) {
			a.print("A");
			b.print("B");
			p.print("P");
			vecAB.print("AB");
			vecAP.print("AP");
			std::cout << "distAB\t" << distAB << "\ndistAP\t" << distAP << "\nAB . AP\t" << ABdotAP << std::endl;
			std::cout << "angle\t" << angleBAP << std::endl;
			std::cout << "distPD\t" << distDP << std::endl;
			std::cout << "Hit\t" << (distDP <= radius) << "\n\n";
		}

		// return if that is within radius of point
		return distDP <= radius;
	}

	bool pointOnRightSizeOfLine(float2 a, float2 b, float2 p, const bool verbose = false)
	{
		float2 vecABperp = perpendicular(vec2d(a, b));
		float2 vecAP = vec2d(a, p);

		if (verbose) {
			float dotProduct = dot(vecABperp, vecAP);

			vec2d(a, b).print("AB:", NO_NEWLINE);
			vecABperp.print("\u27c2AB:", NO_NEWLINE);
			vecAP.print("AP:", NO_NEWLINE);
			printFloatX(dotProduct, "AB \u22c5 AP:", NO_NEWLINE, false);

			if (dotProduct >= 0) {
				std::cout << "\tINSIDE" << std::endl;
			}
		}

		return dot(vecABperp, vecAP) >= 0;
	}

	bool pointInsideTriangle(float2 a, float2 b, float2 c, float2 p, const bool verbose = false)
	{
		if (verbose) { std::cout << "\tSCANNING...\n"; }

		// Sometimes triangles are "inside out" (clockwise points)
		bool insideAB = pointOnRightSizeOfLine(a, b, p, verbose);
		bool insideBC = pointOnRightSizeOfLine(b, c, p, verbose);
		bool insideCA = pointOnRightSizeOfLine(c, a, p, verbose);

		return (insideAB && insideBC && insideCA) || (!insideAB && !insideBC && !insideCA); // FIX THIS!!!
	}
}