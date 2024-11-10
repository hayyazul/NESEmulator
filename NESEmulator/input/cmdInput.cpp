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
	return str;
}

std::string CommandlineInput::getUserLine(std::string msg) {
	std::cout << msg;
	return this->getUserLine();
}

void CommandlineInput::clearExtraneousInput(bool gotChar) {
	std::string dummyStr;
	std::cin.clear();
	if (gotChar) {
		std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
	}  // If the user input any more characters beyond the first one, eliminate those.
}
