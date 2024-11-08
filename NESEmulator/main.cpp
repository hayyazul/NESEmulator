// main.cpp : Defines the entry point for the application.
//

#include "NESEmulator.h"
#include "debuggingTools/testOpcodes.h"

// TODO: 
//  - Add a few other instructions.
//  - Add RAM module.
//  - Implement databus.
//  - Add the rest of the instructions.

int main() {
	NESDebug nes;
	nes.CPU_ptr->powerOn();
	nes.loadROM("testROMS/nestest.nes");
	nes.CPU_ptr->reset();

	// Set the PC to 0xc000 because we have not implemented the PPU yet.
	Registers registers = nes.registersPeek();
	nes.memPoke(0x1f0, 0x801);  // Putting an address between 0x0800 and 0x8000 on the stack.
	registers.PC = 0xc000;
	nes.registersPoke(registers);

	std::cout << "Initial execution address: 0x" << std::hex << (int)nes.CPU_ptr->registersPeek().PC << std::endl;

	for (int i = 0; i < 1000; ++i) {
		if (!nes.executeMachineCycle()) {
			std::cout << "Failure: Invalid opcode: 0x" << std::hex << (int)nes.databus_ptr->read(nes.CPU_ptr->registersPeek().PC) << " at 0x" << std::setfill('0') << std::setw(4) << (int)nes.CPU_ptr->registersPeek().PC << std::endl;
			break;
		}
	}

	std::cout << "Fin" << std::endl;

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