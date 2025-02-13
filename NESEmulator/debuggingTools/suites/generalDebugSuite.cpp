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
	window(SDL_CreateWindow("My Window", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 1024, 480, SDL_WINDOW_RESIZABLE)),
	renderer(SDL_CreateRenderer(window, 0, 0)),
	windowSurface(SDL_GetWindowSurface(window)),
	BLACK(graphics.getRGB(0x00, 0xff, 0xff)), 
	YELLOW(graphics.getRGB(0x00, 0xff, 0xff)) {}
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
		msg = "\n --- What to Perform ---\n - q: Quit\n - e: Execute cycle\n - E [n]: Execute n cycles.\n - b: Display nametable w/ this->graphics.\n - p: Dump PPU internal registers and stored external othis->nes.\n  Your option: ";
		inputChar = this->CLIInputHandler.getUserChar(msg);
		std::cout << std::endl;
		switch (inputChar) {
		case('e'): {

			this->nes.executeMachineCycle();

			if (lastPos.dotInRange(256, 340) || lastPos.lineInRange(240, 261)) {
				this->graphics.drawPixel(BLACK, lastPos.dot, lastPos.scanline);
			}
			PPUPosition pos = this->nes.debugPPU.getPosition();
			lastPos = pos;
			this->graphics.drawPixel(YELLOW, pos.dot, pos.scanline);
			displayPalette(this->graphics, this->nes.debugPPU, 341, 0, 3);
			this->graphics.blitDisplay(this->windowSurface);
			SDL_UpdateWindowSurface(this->window);
			break;
		}
		case('E'): {
			int cyclesToExecute = this->CLIInputHandler.getUserInt("How many cycles?\n");
			if (cyclesToExecute < 0) break;
			std::cout << std::endl;
			PPUPosition pos;
			for (int i = 0; i < cyclesToExecute; ++i) {
				this->nes.executeMachineCycle();
				if (this->nes.frameFinished()) {
					this->graphics.blitDisplay(this->windowSurface);
					SDL_UpdateWindowSurface(this->window);
				}
			}

			break;
		}
		case('b'): {
			NTDisplayer.displayNametable(this->graphics, this->nes.debugPPU, 0, 256, 0, 1, false);
			this->graphics.blitDisplay(this->windowSurface);
			SDL_UpdateWindowSurface(this->window);
			break;
		}
		case('p'): {
			// TODO: Fully implement; currently only outputs control.
			PPUInternals ppuInternals = this->nes.getPPUInternals();
			std::cout << "x: " << displayBinary(ppuInternals.x, 3) <<
				"\nw: " << ppuInternals.w <<
				"\nt: " << displayBinary(ppuInternals.t, 15) <<
				"\nv: " << displayBinary(ppuInternals.v, 15) << std::endl;



			std::cout << "Control: " << displayBinary(ppuInternals.control, 8) << std::endl;
			std::cout << "         -------- \n\
         VPHBSINN \n\
         |||||||| \n\
         ||||||++-Base nametable address \n\
         ||||||    (0 = $2000; 1 = $2400; 2 = $2800; 3 = $2C00) \n\
         |||||+---VRAM address increment per CPU read / write of PPUDATA \n\
         |||||     (0: add 1, going across; 1: add 32, going down) \n\
         ||||+----Sprite pattern table address for 8x8 sprites \n\
         ||||      (0: $0000; 1: $1000; ignored in 8x16 mode) \n\
         |||+-----Background pattern table address(0: $0000; 1: $1000) \n\
         ||+------Sprite size(0: 8x8 pixels; 1: 8x16 pixels – see PPU OAM Byte 1) \n\
         |+-------PPU master / slave select \n\
         |         (0: read backdrop from EXT pins; 1: output color on EXT pins) \n\
         +--------Vblank NMI enable(0: off, 1 : on) \n";

		}
		default:
			break;
		}
	}

	SDL_Quit();

}

void GeneralDebugSuite::updateDisplay() {
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
