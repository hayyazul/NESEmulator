// main.cpp : Defines the entry point for the application.
//

#include "NESEmulator.h"

using ::std::cout;
using ::std::endl;

// TODO: 
//  - Add a few other instructions.
//  - Add RAM module.
//  - Implement databus.
//  - Add the rest of the instructions.

int main() {
//	MyStruct mine;
//	mine.myUnion();

	Memory memory;
	DataBus databus{&memory};
	_6502_CPU cpu{&databus};
	cout << (int)cpu.registersPeek().A << endl;
	cout << (int)cpu.memPeek(0x0000) << endl;
	cout << (int)cpu.memPeek(0x0001) << endl;

	cpu.memPoke(0x0001, 0x10);
	cpu.executeOpcode(0xA9);
	cpu.memPoke(0x00F0, 0xab);
	cpu.memPoke(0x0001, 0xF0);
	cpu.executeOpcode(0x81);
	auto a = cpu.registersPeek().A;
	cout << "Accumulator (expected: 16):" << (int)a << endl;
	auto b = cpu.memPeek(0x00ab);
	cout << "0x00F0 (expected: 16):" << (int)b << endl;

	//cpu.executeOpcode(0x00);

	return 0;
}