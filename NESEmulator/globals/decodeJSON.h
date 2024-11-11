// decodeJson.h - Converts JSON into a map w/ a string as a key, and different items as values. There is probably a library for this,
// but I thought it might be interesting to try and do it on my own
#pragma once
#include <string>
#include <vector>
#include <map>
#include <any>
#include <iostream>
#include <iomanip>

void displayValue(std::any val, int indentationLevel, int level);
void printVector(std::vector<std::any> vec, int indentationLevel, int level);

// Instead of JS for Javascript, we have CPP for C++.
struct CPPON {
	std::map<std::string, std::any> values;
	
	// Attempts to display itself; assumes the following:
	// Has int, char, std::string, or double as primitives OR
	// Has vectors made up of those primitives OR
	// Has JSON structs inside which are also made up of those things.
	inline void print(int indentationLevel, int level = 0) {
		std::cout << std::setfill(' ') << std::setw(level * indentationLevel) << "" << "{" << std::endl;  // Opening brackets;
		bool firstItem = true;
		for (auto& [key, val] : this->values) {
			// First display the key.
			std::cout << std::setfill(' ') << std::setw((level + 1) * indentationLevel) << "" << "\"" << key << "\": ";

			if (!firstItem) {
				std::cout << ",";
				firstItem = false;
			}
			displayValue(val, indentationLevel, level + 1);
			std::cout << std::endl;

		}
		std::cout << std::setfill(' ') << std::setw(level * indentationLevel) << "" << "}";  // Closing brackets.
	};
};

CPPON decodeJSONFile(const char* filepath);
