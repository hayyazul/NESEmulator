#include "NESEmulator.h"

// MAIN TODO: 
// - Connect PPU to Graphics module.
//    - Write a stand-in method to output pixels.
// - Implement PPU Registers.

#include "debuggingTools/suites/basicDebugSuite.hpp"

#include "debuggingTools/NESDebug.h"
#include "debuggingTools/PPUDebug.h"
#include "debuggingTools/debugDisplays/nametableWatch.h"
#include "ppu/ppu.h"

#include "graphics/graphics.h"
#include "graphics/textRenderer.hpp"

#include <SDL.h>


uint32_t rgbaToUint32(uint8_t r, uint8_t g, uint8_t b, uint8_t a) {
	return (a << 24) | (r << 16) | (g << 8) | b;
};

#undef main  // Deals w/ the definition of main in SDL.
int main() { 
	
	Memory VRAM{ 0x800 };
	PPUDebug ppu;

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

//	std::cout << "PPU Cycle Count: " << ppu.cycleCount << std::endl;
	
	//for (int i = 0; i < 4; ++i) {
	//	std::cout << "Nametable " << i << ": " << std::endl << std::endl;
//		ppu.displayNametable(i);
		//std::cout << std::endl << " --- " << std::endl;
	//}
	
	std::array<uint8_t, TABLE_SIZE_IN_BYTES> nametableData = ppu.getNametable(0);

	SDL_Init(SDL_INIT_EVERYTHING);
	Graphics graphics{450, 225};

	SDL_Window* window = SDL_CreateWindow("My Window", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 800, 450, SDL_WINDOW_RESIZABLE);
	SDL_Renderer* renderer = SDL_CreateRenderer(window, 0, 0);
	SDL_Surface* windowSurface = SDL_GetWindowSurface(window);

	graphics.lockDisplay();
	const unsigned int displayScale = 5;
	unsigned int x = 10;
	unsigned int y = 10;
	displayNametable(graphics, ppu, 0, x, y);
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

		//graphics.lockDisplay();
		//graphics.clear();
		//renderText(graphics, "THEQUICKBROWNFOXJUMPEDOVERTHELAZYDOG", 4 + a, 28, 2);
		//renderText(graphics, "0123456789 !,.-", 4, 44 + a, 3);
		//graphics.unlockDisplay();

		SDL_Delay(1.0 / 30.0);
	}

	SDL_Quit();
	//*/

	return 0;
}