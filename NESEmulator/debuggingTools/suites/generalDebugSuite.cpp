#include "generalDebugSuite.h"

#include "../../input/cmdInput.h"
#include "../../debuggingTools/PPUDebug.h"
#include "../../debuggingTools/nesDebug.h"
#include "../../debuggingTools/CPUAnalyzer.h"
#include "../../graphics/graphics.h"
#include "../../debuggingTools/debugDisplays/tableDisplayer.h"
#include "../../debuggingTools/debugDisplays/paletteDisplayer.h"

#include <SDL.h>
#include <minmax.h>
#include <set>
#include <fstream>
#include <filesystem>

/*
Debugging toolset reqs:
 - CLI
	- State machine; given an input, transitions into a new state w/ a new input required, or an end state where the debug action is done and it goes back to querying the user.
 - Abilities
	- Save the internal state of the this->nes to restore at a different time.
	   - Ability to serialize and deserialize this data at a whim.
	- Ability to peek into the internal values (CPU registers, memory, CHRDATA, VRAM, PPU internals, etc.)
	- Ability to modify the internal state at any time.
	   - Modifications can be temporary, but they can also "lock" bits to a certain value.
	- Breakpoints
	   - Can be made on different criteria:
		  - CPU Based
			 - Opcodes
			 - Operands (e.g., 0x06 and 0x20)
			 - PC Address
			 - If in DMA, at the different points in the DMA pipeline.
		  - PPU Based
			 - Beam position
			 - Frame
			 - Sprite
			 - Internal register value (e.g. is x equal to 0b101?)
			 - Sprite evaluation pipeline point (OAM-to-2ndOAM transfers)
			 - 2ndOAM-to-shift register transfers
		  - Can support APU based breakpoints in the future.

*/
// Runs the full debugger program..
// TODO: Split this up into various functions.
// Helper functions
// Updates the output w/ the PPU position indicator.

GeneralDebugSuite::GeneralDebugSuite() : 
	nes(), 
	graphics(514, 262), 
	window(SDL_CreateWindow("My Window", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 1024, 480, SDL_WINDOW_RESIZABLE | SDL_WINDOW_FULLSCREEN_DESKTOP)),
	renderer(SDL_CreateRenderer(window, 0, 0)),
	windowSurface(SDL_GetWindowSurface(window)),
	BLACK(graphics.getRGB(0x00, 0x00, 0x00)), 
	YELLOW(graphics.getRGB(0xff, 0xff, 0x00)),
	WHITE(graphics.getRGB(0xff, 0xff, 0xff)),
	INPUT_OPTIONS({
		{'q', {'q', "Quit"}},
		{'e', {'e', "Execute cycle"} },
		{'E', {'E', "Execute [n] cycles"}},
		{'F', {'F', "Execute till PC is at [x], trying for [n] machine cycles"}},
		{'f', {'f', "Execute till next instruction"}},
		{'p', {'p', "Dump PPU internals (excludes VRAM and CHRDATA)"}},
		{'c', {'c', "Dump CPU internals (excludes RAM and DMA data)"}},
		{'s', {'s', "Save NES internals (includes RAM, VRAM, and DMA data)"}},
		{'t', {'t', "Print available save states (prints the machine cycle associated w/ it)"}},
		{'u', {'u', "Delete a given save state"}},
		{'v', {'v', "Loads a given save state"}},
		{'x', {'x', "Clears the screen"}},
		{'r', {'r', "Poke; change a value in RAM."}},
		})
{}
GeneralDebugSuite::~GeneralDebugSuite() {}

void GeneralDebugSuite::run() {
	
	this->nes.loadROM("testROMS/donkey kong.nes");
	this->nes.powerOn();

	SDL_Init(SDL_INIT_EVERYTHING);
	this->nes.debugPPU.attachGraphics(&this->graphics);

	PatternTableDisplayer PTDisplayer;
	NametableDisplayer NTDisplayer;

	std::cout << "Entering Debugging mode..." << std::endl;
	
	CPUCycleOutcomes cpuCycleOutcome;

	std::string msg;
	bool outputResults = true;
	char inputChar = '0';
	PPUPosition lastPos;
	while (inputChar != 'q') {
		inputChar = this->queryForOption();
		std::cout << std::endl;
		switch (inputChar) {
		case('E'): {
			int cyclesToExecute = this->CLIInputHandler.getUserInt("How many cycles?\n");
			std::cout << std::endl;
			for (int i = 0; i < cyclesToExecute; ++i) {
				this->nes.executeMachineCycle();
				if (this->nes.frameFinished() || i == cyclesToExecute - 1) {
					this->updateDisplay();
				}
			}
			break;
		}
		case('F'): {
			int attemptsAllowed = this->CLIInputHandler.getUserInt("How many machine cycles?\n");
			std::cout << std::endl;
			uint16_t PCBreakPoint = this->CLIInputHandler.getUserHex("PC to stop at?\n");
			std::cout << std::endl;
			for (int i = 0; i < attemptsAllowed; ++i) {
				NESCycleOutcomes outcome = this->nes.executeMachineCycle();
				// Update the display if we finish a frame or execution.
				if (this->nes.debugCPU.pcAt(PCBreakPoint) || this->nes.frameFinished() || i == attemptsAllowed - 1) {
					this->updateDisplay();
				}
				// Check if we reached the breakpoint; inform the user if we did.
				if (this->nes.debugCPU.pcAt(PCBreakPoint)) {
						std::cout << " * Break at " << displayHex(PCBreakPoint, 4) << std::endl;
						break;
				}
				// If we have not reached the breakpoint and we were on our last attempt, we failed to reach it.
				if (i == attemptsAllowed - 1) {
					std::cout << " * Failed to find breakpoint " << displayHex(PCBreakPoint, 4) << std::endl;
				}
			}
			break;
		}
		case('e'): {
			this->nes.executeMachineCycle();
			this->updateDisplay();
			break;
		}
		case('f'): {
			NESCycleOutcomes outcome;
			do {
				outcome = this->nes.executeMachineCycle();
			} while (outcome != INSTRUCTION_AND_PPU_CYCLE && outcome != FAIL_CYCLE);
			this->updateDisplay();
			break;
		}
		case('p'): {
			// TODO: Fully implement; currently only outputs control.
			this->printPPUInternals(this->nes.getPPUInternals());
			break;
		}
		case('c'): {
			this->printCPUInternals(this->nes.getCPUInternals());
			break;
		}
		case('s'): {
			this->saveState();
			break;
		}
		case('t'): {
			this->printSavedStates();
			break;
		}
		case('u'): {
			int idxToDel = this->CLIInputHandler.getUserInt("What index to delete?\n");
			this->deleteSavedState(idxToDel);
			break;
		}
		case('v'): {
			int idxToLoad = this->CLIInputHandler.getUserInt("What index to load?\n");
			this->loadState(idxToLoad);
			break;
		}
		case('x'): {
			
			this->clearDisplay();
			this->updateDisplay();

			// Old function of 'x'
			//int idxToSerialize = this->CLIInputHandler.getUserInt("What index to serialize?\n");
			//this->serializeState(idxToSerialize);
			break;
		}
		case('X'): {
			this->setSaveStateDir();
			break;
		}
		case('z'): {
			this->loadSerializedStates();
			break;
		}
		case('r'): {
			this->pokeRAM();
			break;
		}
		default:
			break;
		}
	}

	SDL_Quit();

}

char GeneralDebugSuite::queryForOption() {
	// Get an input from the user; if it is not valid, then get it again.
	char inputChar = ' ';
	bool choiceMade = false;
	do {
		std::string msg;
		// If this is our first time asking the user, don't say they made an invalid option.
		if (choiceMade) {
			msg = "Invalid Option";
		}
		choiceMade = true;
		msg += "\n --- What to Perform ---\n";
		// Output all options available to the user.
		for (const auto& pair : this->INPUT_OPTIONS) {
			msg += pair.second.format() + "\n";
		}
		msg += "Your option : ";
		inputChar = this->CLIInputHandler.getUserChar(msg);  // Get their input.
	} while (!INPUT_OPTIONS.contains(inputChar));
	
	return inputChar;
}

void GeneralDebugSuite::updateDisplay() {
	bool a, b;
	a = this->lastPos.dotInRange(256, 340);
	b = this->lastPos.lineInRange(240, 261);
	if (this->lastPos.dotInRange(256, 340) || this->lastPos.lineInRange(240, 261)) {
		this->graphics.drawPixel(BLACK, this->lastPos.dot, this->lastPos.scanline);
	}

	// Draw a 2 lines, one to cap off the horizontal drawing space, the other the PPU space.
	for (int i = 0; i < 261; ++i) {
		this->graphics.drawPixel(WHITE, 0x100, i);
	}
	for (int i = 0; i < 261; ++i) {
		this->graphics.drawPixel(WHITE, 341, i);
	}

	PPUPosition pos = this->nes.debugPPU.getPosition();
	this->lastPos = pos;
	this->graphics.drawPixel(YELLOW, pos.dot, pos.scanline);
	displayPalette(this->graphics, this->nes.debugPPU, 341, 0, 3);


	this->graphics.blitDisplay(this->windowSurface);
	SDL_UpdateWindowSurface(this->window);
}

void GeneralDebugSuite::clearDisplay() {
	for (int i = 0; i < this->graphics.w; ++i) {
		for (int j = 0; j < this->graphics.h; ++j) {
			this->graphics.drawPixel(0x000000ff, i, j);
		}
	}
}

void GeneralDebugSuite::pokeRAM() {
	// Query the user for the start and end address (and make sure they are valid).
	int startAddr = this->CLIInputHandler.getUserHex(" * Input a start address (0x000 to 0x7ff): ");
	if (startAddr < 0) {
		std::cout << " * Can not have an address less than 0.\n";
		return;
	} else if (startAddr > 0x7ff) {
		std::cout << " * Can not have an address greater than 0x7ff.\n";
		return;
	}
	// Then a valid end address.
	std::cout << " * Input an end address (" << displayHex(startAddr, 3) << " to 0x7ff): ";
	int endAddr = this->CLIInputHandler.getUserHex();
	if (endAddr < 0) {
		std::cout << " * Can not have an address less than 0.\n";
		return;
	} else if (endAddr > 0x7ff) {
		std::cout << " * Can not have an address greater than 0x7ff.\n";
		return;
	}
	else if (endAddr < startAddr) {
		std::cout << " * Can not have an end address lower than the start address (" << displayHex(startAddr, 3) << ").\n";
		return;
	}

	// Now we get the data to display it to the user.
	std::vector<uint8_t> bytes;
	for (int i = startAddr; i <= endAddr; ++i) {
		bytes.push_back(this->nes.debugRAM.getByte(i));
	}

	// Then we display it.
	std::cout << std::endl;
	displayMemDump(bytes, startAddr, endAddr);
	std::cout << std::endl;

	// Now we ask the user to write in the bytes to rewrite, starting from startAddr.

	for (int i = startAddr; i <= endAddr; ++i) {
		std::cout << " * Insert value (negative to abort further pokes) for address " << displayHex(i, 3) << ": ";
		int byteToWrite = this->CLIInputHandler.getUserHex();
		if (byteToWrite < 0) {
			std::cout << " * Aborting further writes.\n";
			return;
		}

		this->nes.debugRAM.setByte(i, byteToWrite);
	}
}

void GeneralDebugSuite::printPPUInternals(PPUInternals ppuInternals) const {
	const std::array<std::string, 3> PPU_REGISTER_ANNOTATIONS = {
		"         -------- \n\
         VPHBSINN \n\
         |||||||| \n\
         ||||||++- Base nametable address \n\
         ||||||     (0 = $2000; 1 = $2400; 2 = $2800; 3 = $2C00) \n\
         |||||+--- VRAM address increment per CPU read / write of PPUDATA \n\
         |||||      (0: add 1, going across; 1: add 32, going down) \n\
         ||||+---- Sprite pattern table address for 8x8 sprites \n\
         ||||       (0: $0000; 1: $1000; ignored in 8x16 mode) \n\
         |||+----- Background pattern table address(0: $0000; 1: $1000) \n\
         ||+------ Sprite size(0: 8x8 pixels; 1: 8x16 pixels – see PPU OAM Byte 1) \n\
         |+------- PPU master / slave select \n\
         |          (0: read backdrop from EXT pins; 1: output color on EXT pins) \n\
         +-------- Vblank NMI enable(0: off, 1 : on) \n",

		"         -------- \n\
         BGRsbMmG \n\
         |||||||| \n\
         |||||||+- Greyscale(0: normal color, 1 : greyscale) \n\
         ||||||+-- 1: Show background in leftmost 8 pixels of screen, 0 : Hide \n\
         |||||+--- 1 : Show sprites in leftmost 8 pixels of screen, 0 : Hide \n\
         ||||+---- 1 : Enable background rendering \n\
         |||+----- 1 : Enable sprite rendering \n\
         ||+------ Emphasize red \n\
         |+------- Emphasize green \n\
         +-------- Emphasize blue \n",

		"         -------- \n\
         VSOxxxxx \n\
         |||||||| \n\
         |||+++++- (PPU open bus or 2C05 PPU identifier [Emudev Note: This is not emulated.]) \n\
         ||+------ Sprite overflow flag (buggy in real PPUs; this buggyness is emulated) \n\
         |+------- Sprite 0 hit flag \n\
         +-------- Vblank flag, cleared on read (unreliable due to a bug in real PPUs; this is emulated) \n"
	};

	//PPUInternals ppuInternals = this->nes.getPPUInternals();
	
	// Printing the main internals of the PPU.
	std::cout << "Control: " << displayBinary(ppuInternals.control, 8) << std::endl;
	std::cout << PPU_REGISTER_ANNOTATIONS.at(0) << std::endl;
	std::cout << "Mask:    " << displayBinary(ppuInternals.mask, 8) << std::endl;
	std::cout << PPU_REGISTER_ANNOTATIONS.at(1) << std::endl;
	std::cout << "Status:  " << displayBinary(ppuInternals.status, 8) << std::endl;
	std::cout << PPU_REGISTER_ANNOTATIONS.at(2) << std::endl;
	std::cout << "x: " << displayBinary(ppuInternals.x, 3) <<
		"\nw: " << ppuInternals.w <<
		"\nt: " << displayBinary(ppuInternals.t, 15) << " | " << displayHex(ppuInternals.t, 4) <<
		"\nv: " << displayBinary(ppuInternals.v, 15) << " | " << displayHex(ppuInternals.v, 4) << std::endl;
	std::cout << std::dec << "dot: " << ppuInternals.beamPos.dot <<
		"\nline: " << ppuInternals.beamPos.scanline;
	std::cout << "\ncycles: " << ppuInternals.cycleCount << std::endl;

}

void GeneralDebugSuite::printCPUInternals(CPUInternals cpuInternals) {
	// Getting the inernals of the CPU (excludes the DMA, which is not internal to the CPU).
	//CPUInternals cpuInternals = this->nes.getCPUInternals();
	
	// Print everything as hex except the status flag; print that as binary.
	std::cout << "CPU Cycle Count: " << std::dec << cpuInternals.totalCyclesElapsed << std::endl;
	std::cout << "PC: " << displayHex(cpuInternals.registers.PC, 4) << std::endl;
	std::cout << "A:  " << displayHex(cpuInternals.registers.A, 2) << std::endl;
	std::cout << "X:  " << displayHex(cpuInternals.registers.X, 2) << std::endl;
	std::cout << "Y:  " << displayHex(cpuInternals.registers.Y, 2) << std::endl;
	std::cout << "SP: " << displayHex(cpuInternals.registers.SP, 2) << std::endl;
	std::cout << "S: " << displayBinary(cpuInternals.registers.S, 8) << std::endl;
	std::cout << "   -------- \n\
   NV1BDIZC \n\
   |||||||| \n\
   |||||||+- Carry \n\
   ||||||+-- Zero \n\
   |||||+--- Interrupt Disable \n\
   ||||+---- Decimal \n\
   |||+----- (No CPU effect; see: the B flag) \n\
   ||+------ (No CPU effect; always pushed as 1) \n\
   |+------- Overflow \n\
   +-------- Negative" << std::endl;

	/*
	// We will also print the PC vectors. NOTE: These reads are potentially dangerous because read does not guarantee non-modification of any internal values.
	// However, the locations being read SHOULD not affect anything internally.
	std::array<uint16_t, 3> PC_VECTORS;
	PC_VECTORS.at(0) = this->nes.debugDatabus.read(0xfffa) | (this->nes.debugDatabus.read(0xfffb) << 8);
	PC_VECTORS.at(1) = this->nes.debugDatabus.read(0xfffc) | (this->nes.debugDatabus.read(0xfffd) << 8);
	PC_VECTORS.at(2) = this->nes.debugDatabus.read(0xfffe) | (this->nes.debugDatabus.read(0xffff) << 8);
	std::cout << "NMI Vector:   " << displayHex(PC_VECTORS.at(0), 4) << std::endl;
	std::cout << "RESET Vector: " << displayHex(PC_VECTORS.at(1), 4) << std::endl;
	std::cout << "IRQ Vector:   " << displayHex(PC_VECTORS.at(2), 4) << std::endl;
	*/
}

void GeneralDebugSuite::printSavedStates() const {
	for (int i = 0; i < this->saveStates.size(); ++i) {
		NESInternals internals = this->saveStates.at(i);
		std::cout << "    * Save " << std::dec << i + 1 << " Cycles: " << internals.getMachineCycles() << " | " << internals.name << std::endl;
	}
}

void GeneralDebugSuite::deleteSavedState(int idx) {
	// First check if the given index is within bounds.
	if (idx > this->saveStates.size() || idx < 1) {
		std::cout << "    * Invalid index " << std::dec << idx << " for save state vector of size " << this->saveStates.size() << std::endl;
		return;
	}

	// Now erase the given index.
	this->saveStates.erase(this->saveStates.begin() + idx - 1);
}

void GeneralDebugSuite::saveState() {
	std::string name = this->CLIInputHandler.getUserLine(" * Name of save state: ");
	NESInternals internals{ name };
	// We save the internals of the CPU, PPU, RAM, and VRAM.
	internals.cpuInternals = this->nes.getCPUInternals();
	internals.ppuInternals = this->nes.getPPUInternals();
	this->nes.getRAM(internals.ram);
	internals.oamDMAUnit = this->nes.getOAMDMAUnit();

	this->saveStates.push_back(internals);
}

void GeneralDebugSuite::loadState(int idx) {
	// First check if the given index is within bounds.
	if (idx > this->saveStates.size() || idx < 1) {
		std::cout << "    * Invalid index " << std::dec << idx << " for save state vector of size " << this->saveStates.size() << std::endl;
		return;
	}

	// Then load the given state. 
	NESInternals internals = this->saveStates.at(idx - 1);
	this->nes.loadInternals(internals);
}

void GeneralDebugSuite::serializeState(int idx) {
	if (idx > this->saveStates.size() || idx < 1) {
		std::cout << "    * Invalid index " << std::dec << idx << " for save state vector of size " << this->saveStates.size() << std::endl;
		return;
	}
	if (!std::filesystem::is_directory(this->saveStateDir)) {
		std::cout << " * Please input a valid save state directory first.\n";
		return;
	}

	NESInternals internals = this->saveStates.at(idx - 1);

	// Checking if there is a dot in the path, suggesting the user accidentally input a file.
	std::string filename = internals.name + ".nesstate";
	filename = this->saveStateDir + filename;
	std::ofstream file{ filename };
	
	file << this->CURRENT_VERSION << '\n';
	file << internals.getSerialFormat();
}

void GeneralDebugSuite::loadSerializedStates() {  // TODO: COMPLETE
	// First we ask the user if they want to continue after showing them the states in the directory.
	if (!std::filesystem::is_directory(this->saveStateDir)) {
		std::cout << " * Please input a valid save state directory first.\n";
		return;
	}
	
	// First we list all the .nesstate files.
	for (const auto& file : std::filesystem::directory_iterator(this->saveStateDir)) {
		// Check if we are dealing w/ a .nesstate file. If not, skip it.
		std::filesystem::path filepath = file.path().filename();
		std::string filename = filepath.string();
		if (filename.find(".nesstate") == std::string::npos) {
			continue;
		}
		std::cout << filename << std::endl;
	}

	char continueLoading = this->CLIInputHandler.getUserChar(" * Load files (y/n)? (They will be appended to the current vector of save states): ");
	if (continueLoading == 'y') {
		for (const auto& file : std::filesystem::directory_iterator(this->saveStateDir)) {
			// Check if we are dealing w/ a .nesstate file. If not, skip it.
			std::filesystem::path filepath = file.path();
			std::string filename = filepath.filename().string();
			if (filename.find(".nesstate") == std::string::npos) {
				continue;
			}

			NESInternals internals{ filepath };
		}
	}
}

void GeneralDebugSuite::setSaveStateDir() {
	std::string dir = this->CLIInputHandler.getUserLine(" * Input dir (preferrably an absolute path seperated by '/'): ");
	if (std::filesystem::is_directory(dir)) {
		if (dir.back() != '\\') dir += "\\";
		this->saveStateDir = dir;
	}
	else {
		std::cout << " * Invalid path (" << dir << "); not a directory(working dir : " << std::filesystem::current_path() << ")\n";
	}
}

std::string InputOptions::format() const {
	std::string formatted = "";
	// Add some spacing depending on offset.
	for (int i = 0; i < this->offset; ++i) {
		formatted += "   ";
	}
	// Add the bullet point.
	formatted += " - ";
	formatted += this->input;  // Then the input character.
	formatted += ": ";  // Then the input-description divider.
	formatted += this->description;  // Lastly the description.

	return formatted;
}
