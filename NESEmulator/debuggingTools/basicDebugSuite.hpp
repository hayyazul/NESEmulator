#pragma once

#include "NESEmulator.h"
#include "input/cmdInput.h"
#include "debuggingTools/debugDatabus.h"
#include "debuggingTools/NESDebug.h"
#include "debuggingTools/CPUAnalyzer.h"

// Runs the full debugger program..
void debuggingSuite() {
	NESDebug nes;
	CommandlineInput input;
	nes.setStdValue(0xcd);
	nes.loadROM("testROMS/nestest.nes");
	nes.powerOn();

	Registers reg = nes.getCPUPtr()->registersPeek();
	reg.PC = 0xc000;
	nes.getCPUPtr()->registersPoke(reg);
	bool success = true;  // Execute until failure.
	int i = 0;
	nes.setRecord(true);
	while (success && i < 1) {
		success = nes.executeMachineCycle();
		++i;
	}

	const char* filename = "testROMS/nestest.txt";

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
			}
			else {
				std::cout << "Output results OFF" << std::endl;
			}
			break;
		default:
			break;
		}
	}
}