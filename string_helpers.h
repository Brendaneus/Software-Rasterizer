#pragma once
#include <string>
#include <sstream>
#include <vector>

// Splits a string by delimiter
std::vector<std::string> splitStr(const std::string &inputStr, const char delimiter = ' ')
{
	std::stringstream strBuffer(inputStr);

	// Load each substring separated by delimiter into an array
	std::vector<std::string> strList;
	for (std::string subStr; std::getline(strBuffer, subStr, delimiter);) {
		strList.push_back(subStr);
	}

	return strList;
}

// Splits a string by delimiter into an array of ints
std::vector<int> splitStrI(const std::string &inputStr, const char delimiter = ' ') {
	std::stringstream strBuffer(inputStr);

	// Load each substring separated by delimiter into an array
	std::vector<int> intList;
	for (std::string subStr; std::getline(strBuffer, subStr, delimiter);) {
		intList.push_back(std::stoi(subStr));
	}

	return intList;
}

// Splits a string by delimiter into an array of floats
std::vector<float> splitStrF(const std::string &inputStr, const char delimiter = ' ') {
	std::stringstream strBuffer(inputStr);

	// Load each substring separated by delimiter into an array
	std::vector<float> floatList;
	for (std::string subStr; std::getline(strBuffer, subStr, delimiter);) {
		floatList.push_back(std::stof(subStr));
	}

	return floatList;
}