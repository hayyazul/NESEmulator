// parseLogData.h - Parser for some formatted log data. Note that the values are the values BEFORE the opcode was executed.

/*             TODO: Add in the extra missing operand
5001 C6B5 1 D0 05 AA 97 4E A5 F8 14572 128 68
5002 C6BC 0 28 00 AA 97 4E A5 F8 14575 128 77  
  |    |  |  |  |  |  |  |  |  |    |   |   |-- PPU Number 2
  |    |  |  |  |  |  |  |  |  |    |   |------ PPU Number 1
  |    |  |  |  |  |  |  |  |  |    |---------- Cycle Count
  |    |  |  |  |  |  |  |  |  |--------------- SP
  |    |  |  |  |  |  |  |  |------------------ P (S in this implementation)
  |    |  |  |  |  |  |  |--------------------- Y
  |    |  |  |  |  |  |------------------------ X
  |    |  |  |  |  |--------------------------- A
  |    |  |  |  |------------------------------ Operand 1
  |    |  |  |--------------------------------- Opcode
  |    |  |------------------------------------ # of Operands
  |    |--------------------------------------- Program Counter (PC)
  |-------------------------------------------- Order in list (i.e., 5002th opcode executed; indexed from 0)
*/
#pragma once

#include <stdint.h>
#include <iostream>
#include <iomanip>
#include <vector>
#include "../globals/helpers.hpp"
#include "../6502Chip/CPU.h"
#include "../debuggingTools/CPUAnalyzer.h"

/* An entry containing the info regarding the current executed log state.
TODO: Include PPU data.

This is similar to ExecutedInstruction, with the difference that this is NOT meant to be used for undo-ing. The two can be checked
for equality which is useful for assertions.

*/
struct ExecutedOpcodeLogEntry {
	int idx, numOfOperands, cycleCount;
	uint8_t opcode;
	uint8_t operands[2];
	uint8_t ppu[2];
	Registers registers;

	ExecutedOpcodeLogEntry() {}
	ExecutedOpcodeLogEntry(int idx, int numOfOperands, int cycleCount, uint8_t opcode, uint8_t operands[2], uint8_t ppu[2], Registers registers) :
		idx(idx),
		numOfOperands(numOfOperands),
		cycleCount(cycleCount),
		opcode(opcode),
		registers(registers)
	{
		for (int i = 0; i < numOfOperands; ++i) {
			this->operands[i] = operands[i];
		}
		this->ppu[0] = ppu[0];
		this->ppu[1] = ppu[1];
	}
	
	// TODO: Implement PPU checking when implementing the PPU, include operand-inequality failure.
	bool operator==(const ExecutedInstruction& execInstruction) const {
		bool equal = true;
		if (this->cycleCount != execInstruction.lastCycleCount) {
			equal = false;
		}
		if (this->numOfOperands != execInstruction.numOfOperands) {
			equal = false;
		}
		if (this->opcode != execInstruction.opcodeExecuted) {
			equal = false;
		}
		if (this->registers.PC != execInstruction.oldRegisters.PC) {
			equal = false;
		}
		if (this->registers.A != execInstruction.oldRegisters.A) {
			equal = false;
		}
		if (this->registers.X != execInstruction.oldRegisters.X) {
			equal = false;
		}
		if (this->registers.Y != execInstruction.oldRegisters.Y) {
			equal = false;
		}
		if (this->registers.S != execInstruction.oldRegisters.S) {
			equal = false;
		}
		if (this->registers.SP != execInstruction.oldRegisters.SP) {
			equal = false;
		}

		return equal;
	};

	// This prints out a statement declaring the log entry to have the same selected values as the executed instruction, or if they are not equal,
	// to display why. TODO: Include operand-inequality failure statement
	void printEqualityStatement(const ExecutedInstruction& execInstruction) const {
		std::cout << "Reasons for inequality: " << std::endl;
		bool equal = true;
		if (this->cycleCount != execInstruction.lastCycleCount) {
			std::cout << " - The number of cycles in the log (" << std::dec << this->cycleCount << ") fails to match up with the cycles counted (" << execInstruction.lastCycleCount << ")." << std::endl;
			equal = false;
		}
		if (this->numOfOperands != execInstruction.numOfOperands) {
			std::cout << " - The number of operands in the log (" << std::dec << this->numOfOperands << ") fails to match up with the count of the executed instruction (" << execInstruction.numOfOperands << ")." << std::endl;
			equal = false;
		}
		if (this->opcode != execInstruction.opcodeExecuted) {
			std::string logOpcode, execOpcode;  // TODO: rename to instruction
			logOpcode = OPCODE_TO_NAME.at(this->opcode);
			execOpcode = OPCODE_TO_NAME.at(execInstruction.opcodeExecuted);
			std::cout << " - The opcode in the log (" << logOpcode << ") fails to match the one executed (" << execOpcode << ")." << std::endl;
			equal = false;
		}
		if (this->registers.PC != execInstruction.oldRegisters.PC) {
			std::cout << " - The PC in the log (" << displayHex(this->registers.PC, 4) << ") fails to match the one recored (" << displayHex(execInstruction.oldRegisters.PC, 4) << ")." << std::endl;
			equal = false;
		}
		if (this->registers.A != execInstruction.oldRegisters.A) {
			std::cout << " - The A in the log (" << displayHex(this->registers.A, 2) << ") fails to match the one recored (" << displayHex(execInstruction.oldRegisters.A, 2) << ")." << std::endl;
			equal = false;
		}
		if (this->registers.X != execInstruction.oldRegisters.X) {
			std::cout << " - The X in the log (" << displayHex(this->registers.X, 2) << ") fails to match the one recored (" << displayHex(execInstruction.oldRegisters.X, 2) << ")." << std::endl;
			equal = false;
		}
		if (this->registers.Y != execInstruction.oldRegisters.Y) {
			std::cout << " - The Y in the log (" << displayHex(this->registers.Y, 2) << ") fails to match the one recored (" << displayHex(execInstruction.oldRegisters.Y, 2) << ")." << std::endl;
			equal = false;
		}
		if (this->registers.S != execInstruction.oldRegisters.S) {
			std::cout << " - The S in the log (" << displayHex(this->registers.S, 2) << ") fails to match the one recored (" << displayHex(execInstruction.oldRegisters.S, 2) << ")." << std::endl;
			equal = false;
		}
		if (this->registers.SP != execInstruction.oldRegisters.SP) {
			std::cout << " - The SP in the log (" << displayHex(this->registers.SP, 2) << ") fails to match the one recored (" << displayHex(execInstruction.oldRegisters.SP, 2) << ")." << std::endl;
			equal = false;
		}
		if (equal) {
			std::cout << " - The states are equal (this function was erroneously called)." << std::endl;
		}
	}
};

std::vector<ExecutedOpcodeLogEntry> getTestFileLog(const char* filename);