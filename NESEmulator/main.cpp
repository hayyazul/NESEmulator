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
	Graphics graphics{10, 20};

	SDL_Window* window = SDL_CreateWindow("My Window", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 800, 450, SDL_WINDOW_RESIZABLE);
	SDL_Surface* windowSurface = SDL_GetWindowSurface(window);

	graphics.lockDisplay();
	for (int row = 0; row < graphics.h * (3.0 / 4.0); ++row) {
		for (int col = 0; col < graphics.w * (3.0 / 4.0); ++col) {
			graphics.drawPixel((137 * (row * 25 + col) + 26) % 256, 
							   (137 * (row * 25 + col) + 52) % 256, 
							   (137 * (row * 25 + col) + 78) % 256, col, row);
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

		SDL_Delay(1.0 / 60.0);
	}

	SDL_Quit();


	return 0;
}