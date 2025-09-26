// cmdInput.h - An interface to get commandline inputs more easily and more safely.
#pragma once
#include <string>

class CommandlineInput {
public:
	CommandlineInput();
	~CommandlineInput();

	// Asks the user for a single char (0-z); returns that input here.
	char getUserChar();
	char getUserChar(std::string msg);  // Optional message input. Note a new line is NOT placed after delivering the message.

	// Asks the user for a full line of input containing chars.
	std::string getUserLine();
	std::string getUserLine(std::string);  // Optional message input; same as getUserChar.

	// Asking the user for some numerical input; defaults to 0 when invalid.
	int getUserInt();
	int getUserInt(std::string msg);

	int getUserHex();
	int getUserHex(std::string msg);

private:

	// Removes any extra input the user has input (so as to prevent them from affecting future inputs),
	void clearExtraneousInput(bool gotChar);

};