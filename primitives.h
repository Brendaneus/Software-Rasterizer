// Deprecated
#pragma once
#include <iostream> // Debug
#include <vector> // Swap for to std::array for efficiency with static size
#include <string>
//#include <algorithm>
#include <random>
#include "vector_math.h"

using namespace vec;

// Global variables
namespace prmtv
{
	const int WIDTH = 800; // Datatype should be changed to prevent overflow when cast to float
	const int HEIGHT = 600; // Powers of 2 are faster
	const int TOTAL_FRAMES = 1;

	// create 2D array (image) of rbg values with [height, width]
	std::vector<std::vector<float3>> BUFFER(HEIGHT, std::vector<float3>(WIDTH));

	std::vector<float3> verts;
	std::vector<float3> colors;

	std::vector<float2> points;
	std::vector<float> pointRadii;
	std::vector<float2> pointVelocities;
	std::vector<float3> pointCols;
	std::vector<float2> linePoints;
	std::vector<float> lineThicknesses;
	std::vector<float2> linePointVelocities;
	std::vector<float3> lineCols;
	std::vector<float2> triVerts; // sequential list of all vertices of all triangles
	std::vector<float2> triVertVels; // each vertex has its own velocity
	std::vector<float3> triCols; // each triangle as its own color
}


// Moves points with velocity, bouncing off edges of image
void animatePoints(std::vector<float2> &points, std::vector<float2> &pointVelocities, const std::vector<std::vector<float3>> &image) {
	for (int i = 0; i < points.size(); i++) {
		float2 proj = points[i] + pointVelocities[i];

		// Reverse X direction if projected outside width
		if (proj.x > prmtv::WIDTH || proj.x < 0) {
			pointVelocities[i + 0].x *= -1.0f;
		}

		// Reverse Y direction if projected outside height
		if (proj.y > prmtv::HEIGHT || proj.y < 0) {
			pointVelocities[i + 0].y *= -1.0f;
		}

		// move points by their adjusted velocities
		points[i] += pointVelocities[i];
	}
}

// Moves lines with velocity, bouncing off edges of image
void animateLines(std::vector<float2> &linePoints, std::vector<float2> &linePointVelocities, const std::vector<std::vector<float3>> &image) {
	for (int i = 0; i < linePoints.size(); i += 2) {
		float2 aProj = linePoints[i + 0] + linePointVelocities[i + 0];
		float2 bProj = linePoints[i + 1] + linePointVelocities[i + 1];

		// Reverse X direction if outside width (per point)
		if (aProj.x < 0 || aProj.x > prmtv::WIDTH) {
			linePointVelocities[i + 0].x *= -1.0f;
		}
		if (bProj.x < 0 || bProj.x > prmtv::WIDTH) {
			linePointVelocities[i + 1].x *= -1.0f;
		}

		// Reverse Y direction if outside height (per point)
		if (aProj.y < 0 || aProj.y > prmtv::HEIGHT) {
			linePointVelocities[i + 0].y *= -1.0f;
		}
		if (bProj.y < 0 || bProj.y > prmtv::HEIGHT) {
			linePointVelocities[i + 1].y *= -1.0f;
		}

		// move points by their velocities
		linePoints[i + 0] += linePointVelocities[i + 0];
		linePoints[i + 1] += linePointVelocities[i + 1];
	}
}

// Moves triangles with velocity, bouncing off edges of image
void animateTriangles(std::vector<float2> &vertices, std::vector<float2> &velocities, const std::vector<std::vector<float3>> &image) {
	for (int i = 0; i < vertices.size(); i += 3) {
		float2 aProj = vertices[i + 0] + velocities[i + 0];
		float2 bProj = vertices[i + 1] + velocities[i + 1];
		float2 cProj = vertices[i + 2] + velocities[i + 2];

		// Reverse X direction if outside width (per vertex)
		if (aProj.x > prmtv::WIDTH || aProj.x < 0) {
			velocities[i + 0].x *= -1.0f;
		}
		if (bProj.x > prmtv::WIDTH || bProj.x < 0) {
			velocities[i + 1].x *= -1.0f;
		}
		if (cProj.x > prmtv::WIDTH || cProj.x < 0) {
			velocities[i + 2].x *= -1.0f;
		}

		// Reverse Y direction if outside height (per vertex)
		if (aProj.y > prmtv::HEIGHT || aProj.y < 0) {
			velocities[i + 0].y *= -1.0f;
		}
		if (bProj.y > prmtv::HEIGHT || bProj.y < 0) {
			velocities[i + 1].y *= -1.0f;
		}
		if (cProj.y > prmtv::HEIGHT || cProj.y < 0) {
			velocities[i + 2].y *= -1.0f;
		}

		// move vertices by their velocities
		vertices[i + 0] += velocities[i + 0];
		vertices[i + 1] += velocities[i + 1];
		vertices[i + 2] += velocities[i + 2];
	}
}


// Draws points onto image
void drawPoints(const std::vector<float2> &points, const std::vector<float> &pointRadii, const std::vector<float3> &pointCols, std::vector<std::vector<float3>> &image) {
	for (int i = 0; i < points.size(); i++) {
		float2 point = points[i];
		float radius = pointRadii[i];

		// Draw within bounding box of point radius
		float minX = std::max((int)(point.x - radius), 0);
		float minY = std::max((int)(point.y - radius), 0);
		float maxX = std::min((int)(point.x + radius + 1.0f), prmtv::WIDTH);
		float maxY = std::min((int)(point.y + radius + 1.0f), prmtv::HEIGHT);

		// Scan bounding box one pixel at a time
		for (int y = minY; y < maxY; y++) {
			for (int x = minX; x < maxX; x++) {
				float2 pixel = float2(x, y);

				// Color pixel if inside the triangle
				if (pointInsideRadius(point, pixel, radius)) {
					image[y][x] = pointCols[i];
				}
			}
		}
	}
}

// Draws lines onto image
void drawLines(const std::vector<float2> &linePoints, const std::vector<float> &lineThicknesses, const std::vector<float3> &lineCols, std::vector<std::vector<float3>> &image) {
	const float WIDTH_F = (float)prmtv::WIDTH;
	const float HEIGHT_F = (float)prmtv::HEIGHT;

	// Draw 1 line at a time (two points)
	for (int i = 0; i < linePoints.size(); i += 2) {
		float2 pointA = linePoints[i + 0];
		float2 pointB = linePoints[i + 1];
		float thickness = lineThicknesses[i / 2];

		// Draw only within bounding box of line
		float minX = std::clamp(std::min({ pointA.x - thickness * 2, pointB.x - thickness * 2 }), 0.0f, WIDTH_F);
		float minY = std::clamp(std::min({ pointA.y - thickness * 2, pointB.y - thickness * 2 }), 0.0f, HEIGHT_F);
		float maxX = std::clamp(std::max({ pointA.x + thickness * 2, pointB.x + thickness * 2 }), 0.0f, WIDTH_F);
		float maxY = std::clamp(std::max({ pointA.y + thickness * 2, pointB.y + thickness * 2 }), 0.0f, HEIGHT_F);

		// Scan bounding box one pixel at a time
		for (int y = minY; y < maxY; y++) {
			for (int x = minX; x < maxX; x++) {
				float2 pixel = float2(x, y);

				// Color pixel if inside the triangle
				if (lineInterectsRadius(pointA, pointB, pixel, thickness)) {
					image[y][x] = lineCols[i / 2];
				}
			}
		}
	}
}

// Draws triangles onto image
void drawTriangles(const std::vector<float2> &vertices, const std::vector<float3> &triangleCols, std::vector<std::vector<float3>> &image) {
	// Draw 1 triangle at a time (three vertices)
	for (int i = 0; i < vertices.size(); i += 3) {
		float2 vertA = vertices[i + 0];
		float2 vertB = vertices[i + 1];
		float2 vertC = vertices[i + 2];

		// Draw only within bounding box of triangle
		float minX = std::min({ vertA.x, vertB.x, vertC.x });
		float minY = std::min({ vertA.y, vertB.y, vertC.y });
		float maxX = std::max({ vertA.x, vertB.x, vertC.x });
		float maxY = std::max({ vertA.y, vertB.y, vertC.y });

		// Scan bounding box one pixel at a time
		for (int y = minY; y <= maxY; y++) {
			for (int x = minX; x <= maxX; x++) {
				float2 pixel = float2(x, y);

				// Color pixel if inside the triangle
				if (pointInsideTriangle(vertA, vertB, vertC, pixel)) {
					image[y][x] = triangleCols[i / 3];
				}
			}
		}
	}
}


// Creates, renders, and outputs a single image of a fixed point
void createMovingPoints()
{
	const int pointsCount = 250;
	prmtv::points = std::vector<float2>(pointsCount); // sequential list of all vertex of all pointss
	prmtv::pointVelocities = std::vector<float2>(pointsCount); // each point has its own velocity
	prmtv::pointRadii = std::vector<float>(pointsCount); // each point has a radius
	prmtv::pointCols = std::vector<float3>(pointsCount);

	// Set up random generator
	std::random_device rd;
	std::mt19937 gen(rd());
	std::uniform_real_distribution<float> distX(0.0f, (float)prmtv::WIDTH); // float cast necessary here?
	std::uniform_real_distribution<float> distY(0.0f, (float)prmtv::HEIGHT);
	std::uniform_real_distribution<float> distR(0.5f, 3.0f);
	std::uniform_real_distribution<float> distC(0.0f, 255.0f);

	float2 halfSize = float2{ prmtv::WIDTH, prmtv::HEIGHT } / 2.0f;

	// randomly each point's location, velocity, radius, and color
	for (int i = 0; i < prmtv::points.size(); i++) {
		// generate a random point
		prmtv::points[i] = { distX(gen), distY(gen) };
		// generate a random velocity and scale it down
		float2 velocity{ distX(gen), distY(gen) };
		prmtv::pointVelocities[i] = (velocity - halfSize) * 0.01f;
		// generate a random radius
		prmtv::pointRadii[i] = distR(gen);
		// generate a random color
		prmtv::pointCols[i] = float3{ distC(gen), distC(gen), distC(gen) };
	}
}

// Creates, renders, and outputs a single image of a fixed line
void createMovingLines()
{
	const int lineCount = 50;
	prmtv::linePoints = std::vector<float2>(lineCount * 2); // sequential list of all vertex of all triangles
	prmtv::linePointVelocities = std::vector<float2>(prmtv::linePoints.size()); // each point has it's own velocity
	prmtv::lineThicknesses = std::vector<float>(lineCount);
	prmtv::lineCols = std::vector<float3>(lineCount);

	// Set up random generator
	std::random_device rd;
	std::mt19937 gen(rd());
	std::uniform_real_distribution<float> distX(0.0f, (float)prmtv::WIDTH);
	std::uniform_real_distribution<float> distY(0.0f, (float)prmtv::HEIGHT);
	std::uniform_real_distribution<float> distT(0.25f, 1.5f);
	std::uniform_real_distribution<float> distC(0.0f, 255.0f);

	float2 halfSize = float2{ prmtv::WIDTH, prmtv::HEIGHT } / 2.0f;

	// Randomly generate a bunch of points for the lines
	for (int i = 0; i < prmtv::linePoints.size(); i++) {
		float2 pos{ distX(gen), distY(gen) };
		prmtv::linePoints[i] = halfSize + (pos - halfSize) * 0.5f; // scale the distribution
	}

	// Randomly generate a single velocity, thickness, and color for each triangle
	for (int i = 0; i < prmtv::linePointVelocities.size(); i += 2) {
		float2 velocity{ distX(gen), distY(gen) };
		velocity = (velocity - halfSize) * 0.01f; // scale the velocity

		prmtv::lineThicknesses[i / 2] = distT(gen);

		// each point in a line gets the same initial velocity
		prmtv::linePointVelocities[i + 0] = velocity;
		prmtv::linePointVelocities[i + 1] = velocity;

		// each line gets a single random color
		prmtv::lineCols[i / 2] = float3{ distC(gen), distC(gen), distC(gen) };
	}
}

// Creates, renders, and outputs many frames of several moving triangles
void createMovingTriangles() {
	const int triangleCount = 30;
	prmtv::triVerts = std::vector<float2>(triangleCount * 3); // sequential list of all vertex of all triangles
	prmtv::triVertVels = std::vector<float2>(prmtv::triVerts.size()); // each point has it's own velocity
	prmtv::triCols = std::vector<float3>(triangleCount);

	// Set up random generator
	std::random_device rd;
	std::mt19937 gen(rd());
	std::uniform_real_distribution<float> distX(0.0f, (float)prmtv::WIDTH);
	std::uniform_real_distribution<float> distY(0.0f, (float)prmtv::HEIGHT);
	std::uniform_real_distribution<float> distC(0.0f, 255.0f);

	float2 halfSize = float2{ prmtv::WIDTH, prmtv::HEIGHT } / 2.0f;

	// Randomly generate a bunch of vertices for the triangles
	for (int i = 0; i < prmtv::triVerts.size(); i++) {
		float2 pos{ distX(gen), distY(gen) }; // generate
		prmtv::triVerts[i] = halfSize + (pos - halfSize) * 0.3f; // scale the distribution
	}

	// Randomly generate a single velocity and color for each triangle
	for (int i = 0; i < prmtv::triVertVels.size(); i += 3) {
		float2 velocity{ distX(gen), distY(gen) }; // generate
		velocity = (velocity - halfSize) * 0.01f; // scale the velocity
		prmtv::triVertVels[i + 0] = velocity;
		prmtv::triVertVels[i + 1] = velocity; // each vertex in a triangle gets the same initial velocity
		prmtv::triVertVels[i + 2] = velocity;
		// each triangle gets one color
		prmtv::triCols[i / 3] = float3{ distC(gen), distC(gen), distC(gen) };
	}
}