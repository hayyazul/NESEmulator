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
	INPUT_OPTIONS({
		{'q', {'q', "Quit"}},
		{'e', {'e', "Execute cycle"} },
		{'E', {'E', "Execute [n] cycles"}},
		{'p', {'p', "Dump PPU internals (excludes VRAM and CHRDATA)"}},
		{'c', {'c', "Dump CPU internals (excludes RAM and DMA data)"}} 
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

	CPUCycleOutcomes cpuCycleOutcome = FAIL;

	std::cout << "Entering Debugging mode..." << std::endl;

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
		case('e'): {
			this->nes.executeMachineCycle();
			this->updateDisplay();
			break;
		}
		case('p'): {
			// TODO: Fully implement; currently only outputs control.
			this->printPPUInternals();
			break;
		}
		case('c'): {
			this->printCPUInternals();
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
	PPUPosition pos = this->nes.debugPPU.getPosition();
	this->lastPos = pos;
	this->graphics.drawPixel(YELLOW, pos.dot, pos.scanline);
	displayPalette(this->graphics, this->nes.debugPPU, 341, 0, 3);
	this->graphics.blitDisplay(this->windowSurface);
	SDL_UpdateWindowSurface(this->window);
}

void GeneralDebugSuite::printPPUInternals() const {
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
         ||+------ Emphasize red(green on PAL / Dendy) \n\
         |+------- Emphasize green(red on PAL / Dendy) \n\
         +-------- Emphasize blue \n",

		"         -------- \n\
         VSOxxxxx \n\
         |||||||| \n\
         |||+++++- (PPU open bus or 2C05 PPU identifier [Emudev Note: This is not emulated.]) \n\
         ||+------ Sprite overflow flag (buggy in real PPUs; this buggyness is emulated) \n\
         |+------- Sprite 0 hit flag \n\
         +-------- Vblank flag, cleared on read (unreliable due to a bug in real PPUs; this is emulated) \n"
	};

	PPUInternals ppuInternals = this->nes.getPPUInternals();
	
	// Printing the main internals of the PPU.
	std::cout << "Control: " << displayBinary(ppuInternals.control, 8) << std::endl;
	std::cout << PPU_REGISTER_ANNOTATIONS.at(0) << std::endl;
	std::cout << "Mask:    " << displayBinary(ppuInternals.mask, 8) << std::endl;
	std::cout << PPU_REGISTER_ANNOTATIONS.at(1) << std::endl;
	std::cout << "Status:  " << displayBinary(ppuInternals.status, 8) << std::endl;
	std::cout << PPU_REGISTER_ANNOTATIONS.at(2) << std::endl;
	std::cout << "x: " << displayBinary(ppuInternals.x, 3) <<
		"\nw: " << ppuInternals.w <<
		"\nt: " << displayBinary(ppuInternals.t, 15) <<
		"\nv: " << displayBinary(ppuInternals.v, 15) << std::endl;

}

void GeneralDebugSuite::printCPUInternals() {
	// Getting the inernals of the CPU (excludes the DMA, which is not internal to the CPU).
	CPUInternals cpuInternals = this->nes.getCPUInternals();
	
	// Print everything as hex except the status flag; print that as binary.
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

	// We will also print the PC vectors. NOTE: These reads are potentially dangerous because read does not guarantee non-modification of any internal values.
	// However, the locations being read SHOULD not affect anything internally.
	std::array<uint16_t, 3> PC_VECTORS;
	PC_VECTORS.at(0) = this->nes.debugDatabus.read(0xfffa) | (this->nes.debugDatabus.read(0xfffb) << 8);
	PC_VECTORS.at(1) = this->nes.debugDatabus.read(0xfffc) | (this->nes.debugDatabus.read(0xfffd) << 8);
	PC_VECTORS.at(2) = this->nes.debugDatabus.read(0xfffe) | (this->nes.debugDatabus.read(0xffff) << 8);
	std::cout << "NMI Vector:   " << displayHex(PC_VECTORS.at(0), 4) << std::endl;
	std::cout << "RESET Vector: " << displayHex(PC_VECTORS.at(1), 4) << std::endl;
	std::cout << "IRQ Vector:   " << displayHex(PC_VECTORS.at(2), 4) << std::endl;
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
