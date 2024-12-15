// main.cpp : Defines the entry point for the application.
//

#include "NESEmulator.h"

// MAIN TODO: 
// - Implement PPU Registers.

#include "debuggingTools/basicDebugSuite.hpp"

#include "ppu/ppu.h"

#undef main  // Deals w/ the definition of main in SDL.
int main() { 
	
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

	//nes.setRecord(false);

	for (int i = 0; i < 1'875'000; ++i) {
		if (i == 187'000) {
			int a = 0;
		}
		if (!nes.executeMachineCycle()) {
			std::cout << "Illegal opcode encountered!" << std::endl;
			break;
		}
	}
	
	for (int i = 0; i < 4; ++i) {
		std::cout << "Nametable " << i << ": " << std::endl << std::endl;
		ppu.displayNametable(i);
		std::cout << std::endl << " --- " << std::endl;
	} 
//	debuggingSuite();
	return 0;
}