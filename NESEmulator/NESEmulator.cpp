// NESEmulator.cpp : Defines the entry point for the application.
//

#include "NESEmulator.h"
#include "6502Chip/CPU.h"

using ::std::cout;
using ::std::endl;

int main() {
	_6502_CPU cpu;
	cpu.executeCycle();

	cout << "Hello CMake." << endl;
	return 0;
}
