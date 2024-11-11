// main.cpp : Defines the entry point for the application.
//

#include "NESEmulator.h"

// TODO: 
// - Debug instructions
// - Create debugging tools (breakpoints, loggers, deassemblers, etc.)

#include "input/cmdInput.h"
#include "debuggingTools/debugDatabus.h"
#include "debuggingTools/NESDebug.h"
#include "debuggingTools/CPUAnalyzer.h"

#undef main  // Deals w/ the definition of main in SDL.
int main() {
	/*
	DebugDatabus databus;
	databus.setRecordActions(false);  // Disable the recording for now; we do not want to record writes which initialize ROM.
	NES nes{&databus};
	nes.loadROM("testROMS/run.6502.nes");
	databus.setRecordActions(true);  
	for (unsigned int i = 0; i < 10; ++i) {
		nes.executeMachineCycle();
	}
	*/
	Memory memory;
	DebugDatabus databus{&memory};
	DebugDatabus* databusPtr = &databus;
	CPUDebugger cpu{databusPtr};
	cpu.setStdMemValue(0xcd);
	cpu.memPoke(RESET_VECTOR_ADDRESS, 0x00);
	cpu.memPoke(RESET_VECTOR_ADDRESS + 1, 0x02);
	cpu.powerOn();
	// Loading in a basic test program starting from 0x0200:
	uint8_t program[16] = { 
		0xa9, 0x01, 
		0x8d, 0x00, 0x07, 
		0xa9, 0x05, 
		0x8d, 0x01, 0x07, 
		0xa9, 0x08, 
		0x8d, 0x02, 0x07 };
	/*	In assembly:
	LDA #$01
	STA $0700
	LDA #$05
	STA $0701
	LDA #$08
	STA $0702  */
	for (int i = 0; i < 16; ++i) {
		cpu.memPoke(0x0200 + i, program[i]);
	}
	// Dumping the initial values:
	std::vector<uint8_t> testValues = cpu.memDump(0x0700, 0x0705);
	std::cout << "Memory before execution at 0x0700: ";
	for (int i = 0; i < testValues.size(); ++i) { 
		std::cout << "0x" << std::hex << std::setfill('0') << std::setw(2) << (int)testValues.at(i);
		if (i != testValues.size() - 1) {
			std::cout << ", ";
		}
	}
	std::cout << std::endl;
	// Execute program
	for (int i = 0; i < 6; ++i) {
		cpu.executeCycle();
	}
	testValues = cpu.memDump(0x0700, 0x0705);
	std::cout << "Memory after execution at 0x0700: ";
	for (int i = 0; i < testValues.size(); ++i) {
		std::cout << "0x" << std::hex << std::setfill('0') << std::setw(2) << (int)testValues.at(i);
		if (i != testValues.size() - 1) {
			std::cout << ", ";
		}
	}
	std::cout << std::endl;
	// Undo execution (testing out user input as well)
	CommandlineInput input;
	ExecutedInstruction execInstr;
	Registers r;
	execInstr = cpu.getLastExecutedInstruction();
	std::cout << std::setfill('-') << std::setw(20) << '-' << std::endl;
	std::cout << "Last instruction: " << execInstr.instructionName << std::endl;;
	char inputChar = '0';
	while (inputChar != 'q') {
		std::cout << std::setfill('-') << std::setw(20) << '-' << std::endl;
		inputChar = input.getUserChar("What to perform (q: quit, u: undo the last instruction, d: dump registers and 0x0700 to 0x0705): ");
		std::cout << std::endl;
		switch(inputChar) {
		case('u'):
			cpu.undoInstruction();
			execInstr = cpu.getLastExecutedInstruction();
			std::cout << "Last instruction: " << execInstr.instructionName << std::endl;
			break;
		case('d'):
			testValues = cpu.memDump(0x0700, 0x0705);
			r = cpu.registersPeek();
			std::cout << "Register values: A = 0x" << std::hex << std::setfill('0') << std::setw(2) << (int)r.A <<
				", S = 0x" << std::hex << std::setfill('0') << std::setw(2) << (int)r.S <<
				", SP = 0x" << std::hex << std::setfill('0') << std::setw(2) << (int)r.SP <<
				", X = 0x" << std::hex << std::setfill('0') << std::setw(2) << (int)r.X <<
				", Y = 0x" << std::hex << std::setfill('0') << std::setw(2) << (int)r.Y <<
				", PC = 0x" << std::hex << std::setfill('0') << std::setw(4) << (int)r.PC << std::endl;
			std::cout << "Memory (0x0700 to 0x0705 inclusive): ";
			for (int i = 0; i < testValues.size(); ++i) {
				std::cout << "0x" << std::hex << std::setfill('0') << std::setw(2) << (int)testValues.at(i);
				if (i != testValues.size() - 1) {
					std::cout << ", ";
				}
			}
			std::cout << std::endl;
			break;
		default:
			break;
		}
	}

	/*
	NESDebug nes;
	nes.CPU_ptr->powerOn();
	nes.setStdValue(0xcb);  // We set all values to 0xea as to avoid any issues w/ attempting to execute illegal opcodes.
	nes.loadROM("testROMS/nestest.nes");
	//nes.CPU_ptr->reset();

	uint16_t addr;
	bool found = nes.memFind(0x04, addr);
	if (found) {
		std::cout << "0x04 found at address 0x" << std::hex << std::setfill('0') << std::setw(4) << addr << std::endl;
;	} else {
		std::cout << "Failed to find 0x04 in memory." << std::endl;
	
	}

	// Set the PC to 0xc000 because we have not implemented the PPU yet.
	Registers registers = nes.registersPeek();
	nes.memPoke(0x1f0, 0x801);  // Putting an address between 0x0800 and 0x8000 on the stack.
	registers.PC = 0xc000;
	nes.registersPoke(registers);

	std::cout << "Initial execution address: 0x" << std::hex << (int)nes.CPU_ptr->registersPeek().PC << std::endl;

	for (int i = 0; i < 10000; ++i) {
		if (!nes.executeMachineCycle()) {
			std::cout << "Failure: Invalid opcode: 0x" << std::hex << std::setfill('0') << std::setw(2) << (int)nes.databus_ptr->read(nes.CPU_ptr->registersPeek().PC) << " at 0x" << std::setfill('0') << std::setw(4) << (int)nes.CPU_ptr->registersPeek().PC << ", or in the .nes file: 0x" << std::setfill('0') << std::setw(4) << (int)(memAddrToiNESAddr(nes.CPU_ptr->registersPeek().PC)) << std::endl;
			break;
		}
	}

	std::cout << std::hex << "Value of byte 0x2 (Anything but 0x00 is failure): 0x" << std::setfill('0') << std::setw(2) << (int)nes.memPeek(0x2) << std::endl;
	std::cout << std::hex << "Value of byte 0x3 (Anything but 0x00 is failure): 0x" << std::setfill('0') << std::setw(2) << (int)nes.memPeek(0x3) << std::endl;
	
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