#pragma once

#include "../NESDebug.h"
#include "../../graphics/graphics.h"
#include "../../input/cmdInput.h"
#include "../../ppu/ppu.h"

class GeneralDebugSuite {
public:
	GeneralDebugSuite();
	~GeneralDebugSuite();

	void run();

private:
	void updateDisplay();

	// Extra debug variables ("globals")
	PPUPosition lastPos;

	// Necessary variables 
	NESDebug nes;
	Graphics graphics;
	CommandlineInput CLIInputHandler;

	SDL_Window* window;
	SDL_Renderer* renderer;
	SDL_Surface* windowSurface;

	const uint32_t BLACK, YELLOW;
};