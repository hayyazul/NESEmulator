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
	Graphics graphics{ 514, 262 };
	nes.debugPPU.attachGraphics(&graphics);

	SDL_Window* window = SDL_CreateWindow("My Window", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 1024, 480, SDL_WINDOW_RESIZABLE);
	SDL_Renderer* renderer = SDL_CreateRenderer(window, 0, 0);
	SDL_Surface* windowSurface = SDL_GetWindowSurface(window);
	const uint32_t YELLOW = graphics.getRGB(0xff, 0xff, 0);
	const uint32_t BLACK = graphics.getRGB(0, 0, 0);

	PatternTableDisplayer PTDisplayer;
	NametableDisplayer NTDisplayer;

	NESCycleOutcomes success = FAIL_CYCLE;  // Execute until failure.
	CPUCycleOutcomes cpuCycleOutcome = FAIL;
	
	std::cout << "Entering Debugging mode..." << std::endl;

	std::string msg;
	CommandlineInput input;
	bool outputResults = true;
	char inputChar = '0';
	PPUPosition lastPos;
	while (inputChar != 'q') {
		msg = "\n --- What to Perform ---\n - q: Quit\n - e: Execute cycle\n - E [n]: Execute n cycles.\n - b: Display nametable w/ graphics.\n - p: Dump PPU internal registers and stored external ones.\n  Your option: ";
		inputChar = input.getUserChar(msg);
		std::cout << std::endl;
		switch (inputChar) {
		case('e'): {
			
			nes.executeMachineCycle();
			
			if (lastPos.dotInRange(256, 340) || lastPos.lineInRange(240, 261)) {
				graphics.drawPixel(BLACK, lastPos.dot, lastPos.scanline);
			}
			PPUPosition pos = nes.debugPPU.getPosition();
			lastPos = pos;
			graphics.drawPixel(YELLOW, pos.dot, pos.scanline);
			displayPalette(graphics, nes.debugPPU, 341, 0, 3);
			graphics.blitDisplay(windowSurface);
			SDL_UpdateWindowSurface(window);
			break;
		}
		case('E'): {
			int cyclesToExecute = input.getUserInt("How many cycles?\n");
			if (cyclesToExecute < 0) break;
			std::cout << std::endl;
			PPUPosition pos;
			for (int i = 0; i < cyclesToExecute; ++i) {
				nes.executeMachineCycle();
				if (nes.frameFinished()) {
					graphics.blitDisplay(windowSurface);
					SDL_UpdateWindowSurface(window);
				}
			}

			if (lastPos.dotInRange(256, 340) || lastPos.lineInRange(240, 261)) {
				graphics.drawPixel(BLACK, lastPos.dot, lastPos.scanline);
			}
			pos = nes.debugPPU.getPosition();
			lastPos = pos;
			graphics.drawPixel(YELLOW, pos.dot, pos.scanline);
			displayPalette(graphics, nes.debugPPU, 341, 0, 3);
			graphics.blitDisplay(windowSurface);
			SDL_UpdateWindowSurface(window);
			break;
		}
		case('b'): {
			NTDisplayer.displayNametable(graphics, nes.debugPPU, 0, 256, 0, 1, false);
			graphics.blitDisplay(windowSurface);
			SDL_UpdateWindowSurface(window);
			break;
		}
		case('p'): {
			// TODO: Fully implement; currently only outputs control.
			PPUInternals ppuInternals = nes.getPPUInternals();
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