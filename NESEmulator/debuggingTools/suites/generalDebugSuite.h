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

// 
// 
// 
// 
// : abstract away the option and input mechanisms from the debugger singleton.
template <typename INTEGRAL_T>
struct BinarySearchIndices {
public:
	BinarySearchIndices() : lowerIdx(0), middleIdx(0), upperIdx(0) {};
	~BinarySearchIndices() {};

	// Getter functions for the member vars.
	INTEGRAL_T getLowerIdx() const;
	INTEGRAL_T getUpperIdx() const;
	INTEGRAL_T getMiddleIdx() const;  // Gets the middle index of the binary search 
	bool isFinishedWithSearch() const;  // Checks if the search is finished (when upperIdx <= lowerIdx).
	
	// A human-readable view of the current binary search state.
	std::string getPrintableView() const;

	// Setter functions for the variables; also implicitly sets middleIdx.
	void setLowerIdx(INTEGRAL_T idx);
	void setUpperIdx(INTEGRAL_T idx);

	void updateBounds(bool actualIsHigher);  // Updates the indices based on whether to move up or down the binary search.
private:
	INTEGRAL_T lowerIdx, middleIdx, upperIdx;
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
	// Clears the display by setting all pixels to black.
	void clearDisplay();

	// Queries the user for how they want to modify RAM or VRAM.
	void pokeRAM();
	void modifyTileMap();

	// Prints all the internals of the associated component in a formatted way.
	void printPPUInternals(PPUInternals ppuInternals) const;
	void printCPUInternals(CPUInternals cpuInternals);

	// Saves the internal state at the given cycle.
	void printSavedStates() const;
	// Deletes a given save state (if the index exists).
	void deleteSavedState(int idx);

	// Creates a savestate of the NES at the current cycle.
	void saveState();
	// Loads a saveState.
	void loadState(int idx);
	// Serializes a save state. See implementation for more details
	void serializeState(int idx);
	// Loads in save states into the saveStates vector. (WIP)
	void loadSerializedStates();
	// Sets the directory of the save states.
	void setSaveStateDir();

	// Activates or deactivates binary search mode.
	void activateBinSearch(int upperCycleBound, bool CPUBased);
	void performBinarySearchActions();
	void deactivateBinSearch(bool returnToOldState);
		
	const std::string CURRENT_VERSION = "1d1bc25c";

	// Extra debug variables ("globals")
	const std::map<char, InputOptions> INPUT_OPTIONS;  // Set of inputs and their descriptions.
	std::map<char, bool> allowedOptions;  // Input options permitted at the moment.

	std::vector<std::string> debuggerMessages;  // A list of messages to print after printing the options the user may take.
	PPUPosition lastPos;
	std::vector<NESInternals> saveStates;
	std::string saveStateDir;

	// Binary search debugger variables.
	bool inBinSearchMode;
	bool CPUCycleCountBased;
	NESInternals startInternals;  // Save state from when binary search was called; used if the user wants to return to this save state.
	NESInternals lowerCycleInternals;  // Save state from the earlier point (in the binary search process)
	BinarySearchIndices<uint64_t> searchRange;  // The lower, middle, and upper cycle counts to search in.

	// Necessary variables 
	NESDebug nes;
	Graphics graphics;
	CommandlineInput CLIInputHandler;

	SDL_Window* window;
	SDL_Renderer* renderer;
	SDL_Surface* windowSurface;

	const uint32_t BLACK, YELLOW, WHITE, GREEN;
};