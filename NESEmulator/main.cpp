#include "NESEmulator.h"

// MAIN TODO: 
// - Connect PPU to Graphics module.
//    - Write a stand-in method to output pixels.
// - Implement PPU Registers.

#include "debuggingTools/suites/basicDebugSuite.hpp"

#include "debuggingTools/NESDebug.h"
#include "ppu/ppu.h"

#include "graphics/graphics.h"

#include <SDL.h>

#undef main  // Deals w/ the definition of main in SDL.
int main() { 
	/*
	Memory VRAM{ 0x800 };
	PPU ppu;

	NESDatabus databus;
	RAM ram;
	Memory cartirdgeMemory{ 0x10000 };
	_6502_CPU CPU;

	NES nes;
	nes.attachPPU(&ppu);
	nes.attachVRAM(&VRAM);
	nes.attachRAM(&ram);
	nes.attachCartridgeMemory(&cartirdgeMemory);
	nes.loadROM("testROMS/donkey kong.nes");
	nes.powerOn();
	for (int i = 0; i < 3'000'000; ++i) {
		if (i == 600000) {
			int a = 0;
		}
		if (!nes.executeMachineCycle()) {
			std::cout << "Illegal opcode encountered!" << std::endl;
			break;
		}
	}

	std::cout << "PPU Cycle Count: " << ppu.cycleCount << std::endl;
	
	for (int i = 0; i < 4; ++i) {
		std::cout << "Nametable " << i << ": " << std::endl << std::endl;
		ppu.displayNametable(i);
		std::cout << std::endl << " --- " << std::endl;
	}*/

	SDL_Init(SDL_INIT_EVERYTHING);
	Graphics graphics;

	SDL_Window* window = SDL_CreateWindow("My Window", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 1600, 900, 0);
	SDL_Surface* windowSurface = SDL_GetWindowSurface(window);

	graphics.lockDisplay();
	for (int row = 0; row < PICTURE_REGION_HEIGHT; ++row) {
		for (int col = 0; col < PICTURE_REGION_WIDTH; ++col) {
			graphics.drawPixel(100, 0, 100);
		}
	}
	graphics.unlockDisplay();

	bool quit = false;

	int a = 100;
	SDL_Event event;
	while (!quit) {
		while (SDL_PollEvent(&event)) {
			switch(event.type) { 
			case(SDL_QUIT):
				quit = true;
				break;
			default:
				break;
			}
		}

		graphics.blitDisplay(windowSurface);
		SDL_UpdateWindowSurface(window);
		for (int row = 0; row < PICTURE_REGION_HEIGHT; ++row) {
			for (int col = 0; col < PICTURE_REGION_WIDTH; ++col) {
				graphics.drawPixel(a, 0, 100);
			}
		}

		SDL_Delay(200);
	}

	SDL_Quit();


	return 0;
}