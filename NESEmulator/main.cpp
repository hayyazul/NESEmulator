// main.cpp : Defines the entry point for the application.
//

#include "NESEmulator.h"

// MAIN TODO: 
// - Implement PPU Registers.

#include "debuggingTools/suites/ppuDebugSuite.hpp"

#include "debuggingTools/NESDebug.h"
#include "ppu/ppu.h"

#undef main  // Deals w/ the definition of main in SDL.
int main() { /*
	Memory VRAM{ 0x800 };
	PPU ppu;

	NESDatabus databus;
	RAM ram;
	Memory cartirdgeMemory{ 0x10000 };
	_6502_CPU CPU;

	NESDebug nes;
	nes.attachPPU(&ppu);
	nes.attachVRAM(&VRAM);
	nes.attachRAM(&ram);
	nes.attachCartridgeMemory(&cartirdgeMemory);
	nes.loadROM("testROMS/donkey kong.nes");
	nes.powerOn();
	for (int i = 0; i < 30'000'000; ++i) {
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
	} */
	debuggingSuite();
	/*
	NESDebug nes;
	nes.loadROM("testROMS/donkey kong.nes");
	nes.powerOn();
	nes.setRecord(true);
	for (int i = 0; i < 200'000; ++i) {
		if (i == 89'384) {
			int b = 0;
		}
		if (!nes.executeMachineCycle()) {
			std::cout << "Illegal opcode, machine cycle no. " << i << "." << std::endl;
		};
	}
	nes.undoMachineCycle();
	*/
	return 0;
}