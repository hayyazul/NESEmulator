#include "NESEmulator.h"

// MAIN TODO: 
// - Work on the PPU's pixel-by-pixel display.
// - Connect PPU to Graphics module.
// - Implement PPU Registers.

#include "debuggingTools/suites/basicDebugSuite.hpp"

#include "debuggingTools/NESDebug.h"
#include "debuggingTools/PPUDebug.h"
#include "debuggingTools/debugDisplays/tableDisplayer.h"
#include "ppu/ppu.h"

#include "graphics/graphics.h"
#include "graphics/textRenderer.hpp"



#include <SDL.h>

#undef main  // Deals w/ the definition of main in SDL.
int main() { 

	//debuggingSuite();

	///*
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
	for (int i = 0; i < 1'000'000; ++i) {
		if (!nes.executeMachineCycle()) {
			std::cout << "Illegal opcode encountered!" << std::endl;
			break;
		}
	}
	//*/


	/*
	
	PatternTableDisplayer PTDisplayer;
	NametableDisplayer NTDisplayer;
	
	SDL_Init(SDL_INIT_EVERYTHING);
	Graphics graphics{600, 338};

	SDL_Window* window = SDL_CreateWindow("My Window", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 800, 450, SDL_WINDOW_RESIZABLE);
	SDL_Renderer* renderer = SDL_CreateRenderer(window, 0, 0);
	SDL_Surface* windowSurface = SDL_GetWindowSurface(window);

	graphics.lockDisplay();
	// Width in px 
	// (which is equal to the height in px due to this being a square)
	// = width (in patterns) * width (of a pattern) * scale
	// = 16 * 8 * scale = 128 * scale
	bool patternTable = true;
	unsigned int nameTable = 0;
	unsigned int x = 10, y = 10;

	PTDisplayer.displayPatternTable(graphics, ppu, 1, x, y, 1);
	NTDisplayer.displayNametable(graphics, ppu, nameTable, x + 144, y, 1, patternTable);

	graphics.unlockDisplay();

	bool quit = false;

	uint8_t a = 0, oldA = 0;
	SDL_Event event;
	while (!quit) {
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

		graphics.blitDisplay(windowSurface);
		SDL_UpdateWindowSurface(window);

		SDL_Delay(1.0 / 30.0);
	}

	SDL_Quit();
	*/

	return 0;
}