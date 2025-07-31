#pragma once
#include <iostream>
// Print helpers need their own file:

void printFloatY(const float &f, const char *name, const bool &newline = true) {
	if (name) {
		std::cout << name << "\t" << f;
	}
	else {
		std::cout << "\t\t" << f;
	}
	if (newline) { std::cout << '\n'; }
}

void printFloatY(const float &f, const bool &newline = true) {
	std::cout << "\t" << f;
	if (newline) { std::cout << '\n'; }
}

void printFloatX(const float &f, const char *name, const bool &newline = true, const bool &alignY = true) {
	if (name) {
		std::cout << name;
		if (alignY) {
			std::cout << "\t" << f;
		}
		else {
			std::cout << "\t\t" << f;
		}
	}
	else {
		if (alignY) {
			std::cout << "\t\t" << f;
		}
		else {
			std::cout << "\t\t\t" << f;
		}
	}
	if (newline) { std::cout << '\n'; }
}

void printFloatX(const float &f, const bool &newline = true, const bool &alignY = true) {
	if (alignY) {
		std::cout << "\t" << f;
	}
	else {
		std::cout << "\t\t" << f;
	}
	if (newline) { std::cout << '\n'; }
}