#pragma once

#include "../NESDebug.h"
#include "../../graphics/graphics.h"
#include "../../input/cmdInput.h"
#include "../../ppu/ppu.h"

struct InputOptions {
	char input;
	std::string description;
	int offset;

	InputOptions() {};
	InputOptions(char input, std::string desc, int offset=0) : input(input), description(desc), offset(offset) {};
	~InputOptions() {};

	// Returns a formatted version of this which is used when querying the user.
	std::string format() const;
};

// NOTE: For now, this assumes cartridge data is unmodified (so no cartridge RAM).
// This will change once I start working on more advanced mappers.
class GeneralDebugSuite {
public:
	GeneralDebugSuite();
	~GeneralDebugSuite();

	void run();
	
private:
	// Queries the user for an option in the main part of the 
	char queryForOption();

	// Update the display by blitting the screen and position the PPU beam pos marker.
	void updateDisplay();

	// Prints all the internals of the associated component in a formatted way.
	void printPPUInternals() const;
	void printCPUInternals();

	// Extra debug variables ("globals")
	const std::map<char, InputOptions> INPUT_OPTIONS;  // Set of inputs and their descriptions.
	PPUPosition lastPos;

	// Necessary variables 
	NESDebug nes;
	Graphics graphics;
	CommandlineInput CLIInputHandler;

	SDL_Window* window;
	SDL_Renderer* renderer;
	SDL_Surface* windowSurface;

	const uint32_t BLACK, YELLOW;
};