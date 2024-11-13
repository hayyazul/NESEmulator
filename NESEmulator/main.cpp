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
	NESDebug nes;
	CommandlineInput input;
	nes.setStdValue(0xcd);
	nes.loadROM("testROMS/run.6502.nes");
	nes.powerOn();

	Registers reg = nes.getCPUPtr()->registersPeek();
	reg.PC = 0xc000;
	nes.getCPUPtr()->registersPoke(reg);
	bool success = true;  // Execute until failure.
	int i = 0;
	nes.setRecord(false);
	while (success && i < 100) { 		
		success = nes.executeMachineCycle();
		++i;
	}

	CPUDebugger* cpuPtr = nes.getCPUPtr();
	ExecutedInstruction instr = cpuPtr->getLastExecutedInstruction();
	Registers r = cpuPtr->registersPeek();
	int numOfInstr = 0;
	int numOfCycles = 0;
	std::string msg = "";
	std::vector<ExecutedInstruction> lastNInstructions;
	std::vector<uint8_t> memDumpVec;
	uint16_t startAddr, endAddr, addr;


	if (!success) {
		std::cout << "Failure! ";
	}

	std::cout << "Entering Debugging mode..." << std::endl;
	
	
	std::cout << "Last instruction: ";
	instr.print();
	
	// TODO: Create some application class or folder to hold and organize this code.
	bool outputResults = true;
	char inputChar = '0';
	while (inputChar != 'q') {
		std::cout << std::endl << std::setfill('-') << std::setw(20) << '-' << std::endl;
		msg = " --- What to perform --- \n q: quit\n s: see last (n) instructions\n u: undo the last instruction\n     U: undo (n) instructions\n r: dump registers\n d: dump memory from (x) to (y)\n e: execute next instruction\n     E: execute (n) instructions\n     A: execute until (address) or (i) cycles\n o: toggle output; currently (" + btos(outputResults, "ON", "OFF") + ")\n Option: ";
		inputChar = input.getUserChar(msg);
		std::cout << std::endl;
		switch (inputChar) {
		case('u'):
			cpuPtr->undoInstruction();
			std::cout << "Last instruction: " << std::endl;  // Displays the last instruction that was executed (i.e. the one BEFORE the one you just undid).
			instr = cpuPtr->getLastExecutedInstruction();
			instr.print();
			break;
		case('U'):
			msg = "How many: ";
			numOfInstr = abs(input.getUserInt(msg));
			for (int i = 0; i < numOfInstr; ++i) {
				cpuPtr->undoInstruction();
				if (outputResults) {
					std::cout << "(-" << i + 1 << "): ";
					instr = cpuPtr->getLastExecutedInstruction();
					instr.print();
					std::cout << std::endl;
				}
			}
			break;
		case('r'):
			r = cpuPtr->registersPeek();
			std::cout << "Register values: ";
			r.dumpContents();
			std::cout << std::endl;
			break;
		case('s'):
			msg = "How many instructions would you like to see? Give a positive integer value: ";
			numOfInstr = abs(input.getUserInt(msg));
			if (numOfInstr <= 0) {
				break;
			}
			lastNInstructions = cpuPtr->getExecutedInstructions(numOfInstr);
			std::cout << std::endl;
			for (ExecutedInstruction& instruction : lastNInstructions) {
				instruction.print();
				std::cout << std::endl;
			}
			break;
		case('d'):
			msg = "Start address in hex: ";
			startAddr = input.getUserHex(msg);
			msg = "End address in hexS: ";
			endAddr = input.getUserHex(msg);
			memDumpVec = nes.getCPUPtr()->memDump(startAddr, endAddr);
			displayMemDump(memDumpVec, startAddr, endAddr);
			break;
		case('e'):
			success = nes.executeMachineCycle();
			if (!success) {
				std::cout << "Execution failed! For more info, memdump at " << displayHex(nes.getCPUPtr()->registersPeek().PC, 4) << std::endl;
				break;
			}
			else if (outputResults) {
				std::cout << "Instruction executed: " << std::endl;
				nes.getCPUPtr()->getLastExecutedInstruction().print();
			}
			break;
		case('E'):
			msg = "How many: ";
			numOfInstr = input.getUserInt(msg);
			for (int i = 0; i < numOfInstr; ++i) {
				success = nes.executeMachineCycle();
				if (!success) {
					std::cout << "Execution failed! For more info, memdump at " << displayHex(nes.getCPUPtr()->registersPeek().PC, 4) << std::endl;
					break;
				}
				else if (outputResults) {
					std::cout << "(" << std::dec << i << "): ";
					nes.getCPUPtr()->getLastExecutedInstruction().print();
					std::cout << std::endl;
				}
			}
			break;
		case('A'):  // Executes either until the user-given number of cycles is reached OR when the PC lands on the given address (before execution of the opcode at that address).
			msg = "How many cycles before giving up: ";
			numOfCycles = input.getUserInt(msg);
			msg = "Give PC breakpoint value (in hex): ";
			addr = input.getUserHex(msg);
			for (int i = 0; i < numOfCycles && !nes.getCPUPtr()->pcAt(addr); ++i) {
				success = nes.executeMachineCycle();
				if (!success) {
					std::cout << "Execution failed! For more info, memdump at " << displayHex(nes.getCPUPtr()->registersPeek().PC, 4) << std::endl;
					break;
				}
				else if (outputResults) {
					std::cout << "(" << std::dec << i << "): ";
					nes.getCPUPtr()->getLastExecutedInstruction().print();
					std::cout << std::endl;
				}
			}
			break;
		case('o'):  // Toggles output of general-level info. Failures, dumps, and input messages do NOT fall under this category.
			outputResults = !outputResults;
			if (outputResults) {
				std::cout << "Output results ON" << std::endl;
			} else {
				std::cout << "Output results OFF" << std::endl;
			}
		default:
			break;
		}
	}

	//CPUDebuggerTest();

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