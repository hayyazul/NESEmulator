// main.cpp : Defines the entry point for the application.
//

#include "NESEmulator.h"

// TODO: 
//  - Add a few other instructions.
//  - Add RAM module.
//  - Implement databus.
//  - Add the rest of the instructions.

int main() {
	
	NES nes;
	nes.loadROM("testROMS/run.6502.nes");
	for (int i = 0; i < 100; ++i) {
		nes.executeMachineCycle();
	}

	/*
	Memory memory;
	DataBus databus{&memory};
	_6502_CPU cpu{&databus};
	std::cout << (int)cpu.registersPeek().A << std::endl;
	std::cout << (int)cpu.memPeek(0x0000) << std::endl;
	std::cout << (int)cpu.memPeek(0x0001) << std::endl;

	cpu.memPoke(0x0001, 0x10);
	cpu.executeOpcode(0xA9);
	cpu.memPoke(0x00F0, 0xab);
	cpu.memPoke(0x0001, 0xF0);
	cpu.executeOpcode(0x81);
	auto a = cpu.registersPeek().A;
	std::cout << "Accumulator (expected: 16):" << (int)a << std::endl;
	auto b = cpu.memPeek(0x00ab);
	std::cout << "0x00F0 (expected: 16):" << (int)b << std::endl;

	//cpu.executeOpcode(0x00);
	
	*/
	return 0;
}