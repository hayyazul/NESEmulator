#include "NESEmulator.h"

// MAIN TODO: 
// - Fix issues w/ PPU Registers.

#include "debuggingTools/frameCounter.h"
#include "debuggingTools/NESDebug.h"
#include "debuggingTools/PPUDebug.h"
#include "debuggingTools/debugDisplays/tableDisplayer.h"
#include "debuggingTools/debugDisplays/paletteDisplayer.h"
#include "input/cmdInput.h"
#include "ppu/ppu.h"

#include "graphics/graphics.h"
#include "graphics/textRenderer.hpp"


#include <SDL.h>
#include <bitset>

#include "debuggingTools/suites/generalDebugSuite.h"
#include "debuggingTools/debugInput.h"

#include "input/controller.h"

#undef main  // Deals w/ the definition of main in SDL.
int main() { 	
	GeneralDebugSuite g;
	g.run();

	/*
	SDL_Init(SDL_INIT_EVERYTHING);

	SDL_Window* window = SDL_CreateWindow("My Window", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 1024, 480, SDL_WINDOW_RESIZABLE);
	SDL_Renderer* renderer = SDL_CreateRenderer(window, 0, 0);
	SDL_Surface* windowSurface = SDL_GetWindowSurface(window);

	Input input;
	StandardController controller{};
	bool quit = false;
	unsigned long long frameCounter = 0;

	Memory VRAM{ 0x800 };
	PPUDebug ppu;
	NESDatabus databus;
	RAM ram;
	Memory cartridgeMemory{ 0x10000 };
	_6502_CPU CPU;

	NES nes;
	nes.attachCPU(&CPU);
	nes.attachPPU(&ppu);
	nes.attachVRAM(&VRAM);
	nes.attachRAM(&ram);
	nes.attachCartridgeMemory(&cartridgeMemory);
	nes.attachDataBus(&databus);
	nes.attachController(&controller);
	nes.loadROM("testROMS/donkey kong.nes");
	nes.powerOn();


	while (!quit) {
		for (int i = 0; i < 357954; ++++i) {
			nes.executeMachineCycle();
		}

		input.updateInput();
		controller.readInput(input);  // Update the controller every frame.
		quit = input.getQuit();

		controller.setLatch(true);
		controller.readInput(input);
		controller.setLatch(false);

		if (frameCounter % 60 == 0) {
			std::cout << "Button states on frame " << frameCounter << ": \n";
			for (int i = 0; i < 8; ++i) {
				std::cout << controller.getData();
				controller.clock();
			}
			std::cout << std::endl;
		}

		++frameCounter;
		SDL_Delay(1000.0 / 60.0);
	};

	SDL_Quit();
	

	
	GeneralDebugSuite debuggingSuite;
	debuggingSuite.run();
	
	
	Memory VRAM{ 0x800 };
	PPUDebug ppu;
	NESDatabus databus;
	RAM ram;
	Memory cartridgeMemory{ 0x10000 };
	_6502_CPU CPU;

	DebugInput input;
	StandardController controller{};

	NES nes;
	nes.attachCPU(&CPU);
	nes.attachPPU(&ppu);
	nes.attachVRAM(&VRAM);
	nes.attachRAM(&ram);
	nes.attachCartridgeMemory(&cartridgeMemory);
	nes.attachDataBus(&databus);
	nes.attachController(&controller);
	nes.loadROM("testROMS/donkey kong.nes");
	nes.powerOn();
	
	PatternTableDisplayer PTDisplayer;
	NametableDisplayer NTDisplayer;
	CommandlineInput CLI;
	
	SDL_Init(SDL_INIT_EVERYTHING);
	Graphics graphics{514, 256};
	ppu.attachGraphics(&graphics);

	SDL_Window* window = SDL_CreateWindow("My Window", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 1024, 480, SDL_WINDOW_RESIZABLE);
	SDL_Renderer* renderer = SDL_CreateRenderer(window, 0, 0);
	SDL_Surface* windowSurface = SDL_GetWindowSurface(window);

	//graphics.lockDisplay();
	bool patternTable = false;
	unsigned int nameTable = 0;
	unsigned int x = 0, y = 0;

	//graphics.unlockDisplay();

	bool quit = false;

	int numFrames = 1;
	int numElapsed = 0;
	unsigned long long total_frames = 0;
	FrameCounter frame_counter;
	while (!quit) {
		++total_frames;
		for (int i = 0; i < 357954; ++++i) {
			controller.update4021();  // Transfers inputs to 4021 every clock cycle.
			nes.executeMachineCycle();
		}

		input.updateInput();
		/*
		if (total_frames < 1000000) {
			if (numElapsed % 13 == 0) {
				KeyState next_state = input.getKeyState(SDL_SCANCODE_Q) == HELD ? NEUTRAL : HELD;
				input.setKeyState(SDL_SCANCODE_Q, next_state);
			}
		}
		else {
			input.setKeyState(SDL_SCANCODE_RIGHT, HELD);
		}
		

		quit = input.getQuit();
		controller.readInput(input);  // Reads an input every frame
		
		graphics.blitDisplay(windowSurface);
		SDL_UpdateWindowSurface(window);
		
		if (numElapsed >= numFrames) {
			numFrames = CLI.getUserInt("Elapse how many more frames? ");
		}
		if (numElapsed % 60 == 0) {
			std::cout << "\n --- Frame Rate: " << frame_counter.getFrameRate() << " --- \n";
			std::cout << "Frame " << numElapsed << " after pause,\n";
			input.printKeyStates();
		}
		++numElapsed;	
		SDL_Delay(1000.0 / 60.0);
		frame_counter.countFrame();
	}

	SDL_Quit();
	*/
	
	return 0;
}