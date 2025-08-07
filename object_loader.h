// For file parsing, etc.  Will need to be refactored.  (Maybe remodel this as the object file creator)

#pragma once
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include "vector_math.h"
#include "string_helpers.h"

using namespace vec;

// Converts a line of face data into its component index groups
std::vector<std::vector<int>> faceToIndices(const std::string &inputStr) {
	std::vector<std::string> groupStrings = splitStr(inputStr.substr(2));
	
	// Load each index group into an array
	std::vector<std::vector<int>> indexGroups; // indexGroups(groupStrings.size());
	for (int i = 0; i < groupStrings.size(); i++) {
		indexGroups.push_back(splitStrI(groupStrings[i], '/'));
	}
	
	return indexGroups;
}

// Reads .obj file into array of strings, throws an error if load fails
// I feel like a good chunk of this can be abstracted into a splitStr call
std::vector<std::string> readObjFile(const std::string &fileName) {
	std::ifstream file("objects/" + fileName + ".obj");
	if (!file.is_open()) {
		throw std::runtime_error("Failed to open " + fileName);
	}

	// Load entire file into buffer
	std::stringstream strBuffer;
	strBuffer << file.rdbuf();

	// Load each line of file buffer into the output array
	std::vector<std::string> strList;
	for (std::string line; std::getline(strBuffer, line);) {
		strList.push_back(line);
	}

	return strList;
}

// Converts object data into a list of vertices
// File data must be in the correct sequence!
std::vector<float3> loadObjFile(const std::vector<std::string> &objLines)
{
	std::vector<float3> objVertices;
	std::vector<float3> triangleVertices;

	// Load each line into containers depending on type of data
	for (std::string line : objLines) {
		// Vertex positions
		if (line.substr(0, 2) == "v ") {
			// Split vertex line into its component axes (could use some error checking)
			std::vector<float> axes = splitStrF(line.substr(2));
			// Create coordinate from axes, and add that to vertices container
			objVertices.push_back(float3{ axes[0], axes[1], axes[2] });
		}
		// Face indices
		else if (line.substr(0, 2) == "f ") {
			// Load index groups from face line (vertex/texture/normal)
			std::vector<std::vector<int>> faceIndexGroups = faceToIndices(line);

			// Add vertices of each triangle making up face polygon
			for (int i = 0; i < faceIndexGroups.size(); i++) {

				// Each triangle is made of first vertex, previous vertex, and current vertex
				// For triangles additional to first, add first vertex and previous vertex (clockwise) to output list
				if (i >= 3) {
					triangleVertices.push_back(objVertices[ faceIndexGroups[0][0] - 1 ]); // first vertex
					triangleVertices.push_back(objVertices[ faceIndexGroups[i - 1][0] - 1 ]); // previous vertex
				}
				triangleVertices.push_back(objVertices[ faceIndexGroups[i][0] - 1 ]); // current vertex
			}
		}
	}

	return triangleVertices;
}