// NESEmulator.cpp : Defines the entry point for the application.
//

#include "NESEmulator.h"
#include "6502Chip/CPU.h"

using ::std::cout;
using ::std::endl;

// TODO: 
//  - Add a few other instructions.
//  - Add RAM module.
//  - Implement databus.
//  - Add the rest of the instructions.

int main() {
	_6502_CPU cpu;
	cpu.executeCycle();

	return 0;
}
