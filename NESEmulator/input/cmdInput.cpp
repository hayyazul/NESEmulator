#include "cmdInput.h"
#include <iostream>
#include <limits>

CommandlineInput::CommandlineInput() {
}

CommandlineInput::~CommandlineInput() {
}

char CommandlineInput::getUserChar() {
	char c;
	std::cin >> c;
	this->clearExtraneousInput(true);
	return c;
}

char CommandlineInput::getUserChar(std::string msg) {
	std::cout << msg;
	return this->getUserChar();
}

std::string CommandlineInput::getUserLine() {
	std::string str, dummyStr;
	std::getline(std::cin, str);
	this->clearExtraneousInput(false);

	// Check if a string has been read properly. If not, return 0 by default.
	if (str.size() < 1) {
		return "0";
	}

	return str;
}

std::string CommandlineInput::getUserLine(std::string msg) {
	std::cout << msg;
	return this->getUserLine();
}

int CommandlineInput::getUserInt() {
	std::string userInput = this->getUserLine();  // Get the input.
	bool firstChar = true;
	for (const char& c : userInput) {  // Check if it is a valid input (e.g., the first character is a number or a dash; all subsequent characters are numbers)
		if (!(isdigit(c) || (firstChar && c == '-'))) {
			return 0;  // Default to 0 in this case.
		}
	}

	if (userInput == "") {
		return 0;
	}

	return stoi(userInput);
}

int CommandlineInput::getUserInt(std::string msg) {
	std::cout << msg;
	return this->getUserInt();
}

int CommandlineInput::getUserHex() {
	std::string userInput = this->getUserLine();  // Get the input.
	std::string hexStr = "";
	bool negative = false;
	// Check if we have a negative input.
	if (userInput.at(0) == '-') {
		negative = true;
	}

	// Basic error checking.
	if (userInput.size() < (3 + negative) || userInput.at(1 + negative) != 'x') {
		return 0;
	}

	int i = 0;
	for (const char& c : userInput) {  
		if (i < (2 + negative)) {  // Skip over the "0x" prefix. 
			++i;
			continue;
		}

		if (!isxdigit(c)) {
			return 0;  // Default to 0 in this case.
		} 
		hexStr += c;
	}

	return (1 - (negative * 2)) * stoi(hexStr, 0, 16);
}

int CommandlineInput::getUserHex(std::string msg) {
	std::cout << msg;
	return this->getUserHex();
}

void CommandlineInput::clearExtraneousInput(bool gotChar) {
	std::string dummyStr;
	std::cin.clear();
	if (gotChar) {
		std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
	}  // If the user input any more characters beyond the first one, eliminate those.
}
