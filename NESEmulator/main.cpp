#include "NESEmulator.h"

// MAIN TODO: 
// - Work on the PPU's pixel-by-pixel display.
// - Connect PPU to Graphics module.
// - Fix issues w/ PPU Registers.

#include "debuggingTools/suites/basicDebugSuite.hpp"

#include "debuggingTools/NESDebug.h"
#include "debuggingTools/PPUDebug.h"
#include "debuggingTools/debugDisplays/tableDisplayer.h"
#include "debuggingTools/debugDisplays/paletteDisplayer.h"
#include "ppu/ppu.h"

#include "graphics/graphics.h"
#include "graphics/textRenderer.hpp"


#include <SDL.h>
#include <bitset>

#undef main  // Deals w/ the definition of main in SDL.
int main() { 

	//debuggingSuite();

	
	Memory VRAM{ 0x800 };
	PPUDebug ppu;
	NESDatabus databus;
	RAM ram;
	Memory cartridgeMemory{ 0x10000 };
	_6502_CPU CPU;

	NES nes;
	nes.attachPPU(&ppu);
	nes.attachVRAM(&VRAM);
	nes.attachRAM(&ram);
	nes.attachCartridgeMemory(&cartridgeMemory);
	nes.loadROM("testROMS/donkey kong.nes");
	nes.powerOn();
	//for (int i = 0; i < 3'000'000; ++i) {
		//if (!nes.executeMachineCycle()) {
			//std::cout << "Illegal opcode encountered!" << std::endl;
			//break;
		//}
	//}

	
	ppu.dumpOAMData(16);

	PatternTableDisplayer PTDisplayer;
	NametableDisplayer NTDisplayer;
	
	SDL_Init(SDL_INIT_EVERYTHING);
	Graphics graphics{514, 256};
	ppu.attachGraphics(&graphics);

	SDL_Window* window = SDL_CreateWindow("My Window", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 1024, 480, SDL_WINDOW_RESIZABLE);
	SDL_Renderer* renderer = SDL_CreateRenderer(window, 0, 0);
	SDL_Surface* windowSurface = SDL_GetWindowSurface(window);

	for (int i = 0; i < 3570954; ++++i) {
		nes.executeMachineCycle();
	}

	//graphics.lockDisplay();
	// Width in px 
	// (which is equal to the height in px due to this being a square)
	// = width (in patterns) * width (of a pattern) * scale
	// = 16 * 8 * scale = 128 * scale
	bool patternTable = true;
	unsigned int nameTable = 0;
	unsigned int x = 0, y = 0;

	PTDisplayer.displayPatternTable(graphics, ppu, patternTable, x + 256, y, 2);
	//NTDisplayer.displayNametable(graphics, ppu, nameTable, x + 256, y, 1, patternTable);
	displayPalette(graphics, ppu, 256, 240, 8);
	//graphics.drawSquare(0xffffffff, (x + 288) + 0x38, (y) + 0x7f, 8);

	//graphics.unlockDisplay();

	bool quit = false;

	uint8_t a = 0, oldA = 0;
	SDL_Event event;
	while (!quit) {
		for (int i = 0; i < 357954; ++++i) {
			quit = !nes.executeMachineCycle();
		}

		while (SDL_PollEvent(&event)) {
			switch(event.type) { 
			case(SDL_QUIT):
				quit = true;
				break;
			case(SDL_WINDOWEVENT):
				if (event.window.event == SDL_WINDOWEVENT_RESIZED) {  // Resizing a window invalidates its surface, so we get the surface again.
					SDL_DestroyWindowSurface(window);
					windowSurface = SDL_GetWindowSurface(window);
				}
				break;
			default:
				break;
			}
		}

		PTDisplayer.displayPatternTable(graphics, ppu, patternTable, x + 256, y, 2);
		displayPalette(graphics, ppu, 0, 240, 8);
		graphics.blitDisplay(windowSurface);
		SDL_UpdateWindowSurface(window);

		SDL_Delay(1.0 / 60.0);
	}

	SDL_Quit();
	//*/

	return 0;
}