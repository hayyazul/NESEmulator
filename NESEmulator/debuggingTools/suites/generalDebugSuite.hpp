#pragma once

#include "input/cmdInput.h"
#include "debuggingTools/PPUDebug.h"
#include "debuggingTools/NESDebug.h"
#include "debuggingTools/CPUAnalyzer.h"
#include "graphics/graphics.h"
#include "debuggingTools/debugDisplays/tableDisplayer.h"
#include "debuggingTools/debugDisplays/paletteDisplayer.h"

#include <SDL.h>
#include <minmax.h>

// Runs the full debugger program..
// TODO: Split this up into various functions.
void debuggingSuite() {
	NESDebug nes{};
	
	nes.loadROM("testROMS/donkey kong.nes");
	nes.powerOn();
	
	SDL_Init(SDL_INIT_EVERYTHING);
	Graphics graphics{ 514, 256 };
	nes.debugPPU.attachGraphics(&graphics);

	SDL_Window* window = SDL_CreateWindow("My Window", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 1024, 480, SDL_WINDOW_RESIZABLE);
	SDL_Renderer* renderer = SDL_CreateRenderer(window, 0, 0);
	SDL_Surface* windowSurface = SDL_GetWindowSurface(window);

	PatternTableDisplayer PTDisplayer;
	NametableDisplayer NTDisplayer;

	NESCycleOutcomes success = FAIL_CYCLE;  // Execute until failure.
	CPUCycleOutcomes cpuCycleOutcome = FAIL;
	
	std::cout << "Entering Debugging mode..." << std::endl;

	std::string msg;
	CommandlineInput input;
	bool outputResults = true;
	char inputChar = '0';
	while (inputChar != 'q') {
		msg = "\n --- What to Perform ---\n - q: Quit\n - e: Execute cycle\n - E [n]: Execute n cycles.\n - b: Display nametable w/ graphics.\n  Your option: ";
		inputChar = input.getUserChar(msg);
		std::cout << std::endl;
		switch (inputChar) {
		case('e'): {
			nes.executeMachineCycle();
			break;
		}
		case('E'): {
			int cyclesToExecute = input.getUserInt("How many cycles?\n");
			if (cyclesToExecute < 0) break;
			std::cout << std::endl;
			for (int i = 0; i < cyclesToExecute; ++i) {
				nes.executeMachineCycle();
				if (nes.frameFinished()) {
					graphics.blitDisplay(windowSurface);
					SDL_UpdateWindowSurface(window);
				}
			}
			break;
		}
		case('b'): {
			NTDisplayer.displayNametable(graphics, nes.debugPPU, 0, 256, 0, 1, false);
			graphics.blitDisplay(windowSurface);
			SDL_UpdateWindowSurface(window);
			break;
		}
		default:
			break;
		}
	}

	SDL_Quit();
}