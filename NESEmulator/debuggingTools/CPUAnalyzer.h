// CPUAnalyzer.h - A set of tools designed to work with the debugger version of the NES class (NESDebug) in order to streamline
// debugging and analysis of the CPU.
#pragma once

#include "../6502Chip/CPU.h"
#include "../globals/helpers.hpp"
#include "debugDatabus.h"

#include <string>
#include <map>
#include <vector>

#include <iostream>
#include <iomanip>



// Map between bytes and their associated instructions as strings.
const std::map<uint8_t, std::string> opcodeToName = {
{0x00, "BRK IMPLD"},
{0x01, "ORA INDXD"},
{0x02, "??? ?????"},
{0x03, "??? ?????"},
{0x04, "??? ?????"},
{0x05, "ORA ZEROP"},
{0x06, "ASL ZEROP"},
{0x07, "??? ?????"},
{0x08, "PHP IMPLD"},
{0x09, "ORA IMMED"},
{0x0A, "ASL ACCUM"},
{0x0B, "??? ?????"},
{0x0C, "??? ?????"},
{0x0D, "ORA ABSLT"},
{0x0E, "ASL ABSLT"},
{0x0F, "??? ?????"},
{0x10, "BPL RELTV"},
{0x11, "ORA INDXY"},
{0x12, "??? ?????"},
{0x13, "??? ?????"},
{0x14, "??? ?????"},
{0x15, "ORA ZEROX"},
{0x16, "ASL ZEROX"},
{0x17, "??? ?????"},
{0x18, "CLC IMPLD"},
{0x19, "ORA ABSY_"},
{0x1A, "??? ?????"},
{0x1B, "??? ?????"},
{0x1C, "??? ?????"},
{0x1D, "ORA ABSX_"},
{0x1E, "ASL ABSX_"},
{0x1F, "??? ?????"},
{0x20, "JSR ABSLT"},
{0x21, "AND INDXD"},
{0x22, "??? ?????"},
{0x23, "??? ?????"},
{0x24, "BIT ZEROP"},
{0x25, "AND ZEROP"},
{0x26, "ROL ZEROP"},
{0x27, "??? ?????"},
{0x28, "PLP IMPLD"},
{0x29, "AND IMMED"},
{0x2A, "ROL ACCUM"},
{0x2B, "??? ?????"},
{0x2C, "BIT ABSLT"},
{0x2D, "AND ABSLT"},
{0x2E, "ROL ABSLT"},
{0x2F, "??? ?????"},
{0x30, "BMI RELTV"},
{0x31, "AND INDXY"},
{0x32, "??? ?????"},
{0x33, "??? ?????"},
{0x34, "??? ?????"},
{0x35, "AND ZEROX"},
{0x36, "ROL ZEROX"},
{0x37, "??? ?????"},
{0x38, "SEC IMPLD"},
{0x39, "AND ABSY_"},
{0x3A, "??? ?????"},
{0x3B, "??? ?????"},
{0x3C, "??? ?????"},
{0x3D, "AND ABSX_"},
{0x3E, "ROL ABSX_"},
{0x3F, "??? ?????"},
{0x40, "RTI IMPLD"},
{0x41, "EOR INDXD"},
{0x42, "??? ?????"},
{0x43, "??? ?????"},
{0x44, "??? ?????"},
{0x45, "EOR ZEROP"},
{0x46, "LSR ZEROP"},
{0x47, "??? ?????"},
{0x48, "PHA IMPLD"},
{0x49, "EOR IMMED"},
{0x4A, "LSR ACCUM"},
{0x4B, "??? ?????"},
{0x4C, "JMP ABSLT"},
{0x4D, "EOR ABSLT"},
{0x4E, "LSR ABSLT"},
{0x4F, "??? ?????"},
{0x50, "BVC RELTV"},
{0x51, "EOR INDXY"},
{0x52, "??? ?????"},
{0x53, "??? ?????"},
{0x54, "??? ?????"},
{0x55, "EOR ZEROX"},
{0x56, "LSR ZEROX"},
{0x57, "??? ?????"},
{0x58, "CLI IMPLD"},
{0x59, "EOR ABSY_"},
{0x5A, "??? ?????"},
{0x5B, "??? ?????"},
{0x5C, "??? ?????"},
{0x5D, "EOR ABSX_"},
{0x5E, "LSR ABSX_"},
{0x5F, "??? ?????"},
{0x60, "RTS IMPLD"},
{0x61, "ADC INDXD"},
{0x62, "??? ?????"},
{0x63, "??? ?????"},
{0x64, "??? ?????"},
{0x65, "ADC ZEROP"},
{0x66, "ROR ZEROP"},
{0x67, "??? ?????"},
{0x68, "PLA IMPLD"},
{0x69, "ADC IMMED"},
{0x6A, "ROR ACCUM"},
{0x6B, "??? ?????"},
{0x6C, "JMP INDRT"},
{0x6D, "ADC ABSLT"},
{0x6E, "ROR ABSLT"},
{0x6F, "??? ?????"},
{0x70, "BVS RELTV"},
{0x71, "ADC INDXY"},
{0x72, "??? ?????"},
{0x73, "??? ?????"},
{0x74, "??? ?????"},
{0x75, "ADC ZEROX"},
{0x76, "ROR ZEROX"},
{0x77, "??? ?????"},
{0x78, "SEI IMPLD"},
{0x79, "ADC ABSY_"},
{0x7A, "??? ?????"},
{0x7B, "??? ?????"},
{0x7C, "??? ?????"},
{0x7D, "ADC ABSX_"},
{0x7E, "ROR ABSX_"},
{0x7F, "??? ?????"},
{0x80, "??? ?????"},
{0x81, "STA INDXD"},
{0x82, "??? ?????"},
{0x83, "??? ?????"},
{0x84, "STY ZEROP"},
{0x85, "STA ZEROP"},
{0x86, "STX ZEROP"},
{0x87, "??? ?????"},
{0x88, "DEY IMPLD"},
{0x89, "??? ?????"},
{0x8A, "TXA IMPLD"},
{0x8B, "??? ?????"},
{0x8C, "STY ABSLT"},
{0x8D, "STA ABSLT"},
{0x8E, "STX ABSLT"},
{0x8F, "??? ?????"},
{0x90, "BCC RELTV"},
{0x91, "STA INDXY"},
{0x92, "??? ?????"},
{0x93, "??? ?????"},
{0x94, "STY ZEROX"},
{0x95, "STA ZEROX"},
{0x96, "STX ZEROY"},
{0x97, "??? ?????"},
{0x98, "TYA IMPLD"},
{0x99, "STA ABSY_"},
{0x9A, "TXS IMPLD"},
{0x9B, "??? ?????"},
{0x9C, "??? ?????"},
{0x9D, "STA ABSX_"},
{0x9E, "??? ?????"},
{0x9F, "??? ?????"},
{0xA0, "LDY IMMED"},
{0xA1, "LDA INDXD"},
{0xA2, "LDX IMMED"},
{0xA3, "??? ?????"},
{0xA4, "LDY ZEROP"},
{0xA5, "LDA ZEROP"},
{0xA6, "LDX ZEROP"},
{0xA7, "??? ?????"},
{0xA8, "TAY IMPLD"},
{0xA9, "LDA IMMED"},
{0xAA, "TAX IMPLD"},
{0xAB, "??? ?????"},
{0xAC, "LDY ABSLT"},
{0xAD, "LDA ABSLT"},
{0xAE, "LDX ABSLT"},
{0xAF, "??? ?????"},
{0xB0, "BCS RELTV"},
{0xB1, "LDA INDXY"},
{0xB2, "??? ?????"},
{0xB3, "??? ?????"},
{0xB4, "LDY ZEROX"},
{0xB5, "LDA ZEROX"},
{0xB6, "LDX ZEROY"},
{0xB7, "??? ?????"},
{0xB8, "CLV IMPLD"},
{0xB9, "LDA ABSY_"},
{0xBA, "TSX IMPLD"},
{0xBB, "??? ?????"},
{0xBC, "LDY ABSX_"},
{0xBD, "LDA ABSX_"},
{0xBE, "LDX ABSY_"},
{0xBF, "??? ?????"},
{0xC0, "CPY IMMED"},
{0xC1, "CMP INDXD"},
{0xC2, "??? ?????"},
{0xC3, "??? ?????"},
{0xC4, "CPY ZEROP"},
{0xC5, "CMP ZEROP"},
{0xC6, "DEC ZEROP"},
{0xC7, "??? ?????"},
{0xC8, "INY IMPLD"},
{0xC9, "CMP IMMED"},
{0xCA, "DEX IMPLD"},
{0xCB, "??? ?????"},
{0xCC, "CPY ABSLT"},
{0xCD, "CMP ABSLT"},
{0xCE, "DEC ABSLT"},
{0xCF, "??? ?????"},
{0xD0, "BNE RELTV"},
{0xD1, "CMP INDXY"},
{0xD2, "??? ?????"},
{0xD3, "??? ?????"},
{0xD4, "??? ?????"},
{0xD5, "CMP ZEROX"},
{0xD6, "DEC ZEROX"},
{0xD7, "??? ?????"},
{0xD8, "CLD IMPLD"},
{0xD9, "CMP ABSY_"},
{0xDA, "??? ?????"},
{0xDB, "??? ?????"},
{0xDC, "??? ?????"},
{0xDD, "CMP ABSX_"},
{0xDE, "DEC ABSX_"},
{0xDF, "??? ?????"},
{0xE0, "CPX IMMED"},
{0xE1, "SBC INDXD"},
{0xE2, "??? ?????"},
{0xE3, "??? ?????"},
{0xE4, "CPX ZEROP"},
{0xE5, "SBC ZEROP"},
{0xE6, "INC ZEROP"},
{0xE7, "??? ?????"},
{0xE8, "INX IMPLD"},
{0xE9, "SBC IMMED"},
{0xEA, "NOP IMPLD"},
{0xEB, "??? ?????"},
{0xEC, "CPX ABSLT"},
{0xED, "SBC ABSLT"},
{0xEE, "INC ABSLT"},
{0xEF, "??? ?????"},
{0xF0, "BEQ RELTV"},
{0xF1, "SBC INDXY"},
{0xF2, "??? ?????"},
{0xF3, "??? ?????"},
{0xF4, "??? ?????"},
{0xF5, "SBC ZEROX"},
{0xF6, "INC ZEROX"},
{0xF7, "??? ?????"},
{0xF8, "SED IMPL_"},
{0xF9, "SBC ABSY_"},
{0xFA, "??? ?????"},
{0xFB, "??? ?????"},
{0xFC, "??? ?????"},
{0xFD, "SBC ABSX_"},
{0xFE, "INC ABSX_"},
{0xFF, "??? ?????"} };

// Contains basic info regarding the execution of the most recent instruction.
struct ExecutedInstruction {
	// We record both the byte and the instruction just in case an opcode was assigned to the wrong instruction.

	std::string instructionName;
	uint8_t opcodeExecuted;  
	Instruction* instructionExecuted;  // This will also contain other important info, like how many operands were involved.
	unsigned int executedIndex;  // The index of when this execution was executed. 

	int numOfOperands, numOfCycles, lastCycleCount;
	uint8_t operands[2];  // The upto two operands involved in the instruction. Default values are 0.
	

	// OLD NOTE: I might not need to access the DatabusAction(s) located in here directly; I could just get the size, then ask
	// the databus to undo [size] number of memory operations. This vector might be removed, though I also might keep it for 
	// other debugging purposes.
	//std::vector<DatabusAction> actionsInvolved;  // Also record the memory actions involved; useful for undoing an instruction.
	unsigned int numOfActionsInvolved;
	Registers oldRegisters;

	ExecutedInstruction() {};
	ExecutedInstruction(uint8_t opcode, Instruction* instruction, unsigned int numActions, Registers registers, uint8_t operands[2], unsigned int idx, int lastCycleCount) :
		opcodeExecuted(opcode),
		instructionExecuted(instruction),
		numOfActionsInvolved(numActions),
		oldRegisters(registers),
		numOfOperands(instruction->numBytes - 1),
		executedIndex(idx),
		lastCycleCount(lastCycleCount)
	{
		this->instructionName = opcodeToName.at(opcode);
		this->numOfCycles = instruction->cycleCount;
		this->operands[0] = operands[0];
		this->operands[1] = operands[1];
	};
	~ExecutedInstruction() {};

	// NOTE: I might return a string containing the message instead.
	// Outputs the instruction executed, the operands, and the values of the registers BEFORE execution.
	void print() {
		// JMP ABSLT | Operands: 0x02, 0x04 | Old Values of A: 0x00, X: 0x02, Y:0x09, SP: 0xf3, PC: 0x0110 | Flags C:0 Z:0 I:0 D:0 V:0 N:0
		std::cout << this->instructionName << " | ";  // Opcode name
		std::cout << "Operands: ";  // Now the operands
		for (unsigned int i = 0; i < 2; ++i) {
			if (i < this->numOfOperands) {
				std::cout << displayHex(this->operands[i], 2);
			}
			else {
				std::cout << "____";
			}

			if (i == 0) {
				std::cout << ", ";
			}
		}
		// Now the registers (excluding flags)
		std::cout << " | Old values of A: " << displayHex(oldRegisters.A, 2)
			<< ", X: " << displayHex(oldRegisters.X, 2)
			<< ", Y: " << displayHex(oldRegisters.Y, 2)
			<< ", SP: " << displayHex(oldRegisters.SP, 2)
			<< ", PC: " << displayHex(oldRegisters.PC, 4) << " | Flags " << displayHex(oldRegisters.S, 2) << " C:";

		// Lastly, the flags.
		std::cout << oldRegisters.getStatus('C')
			<< ", Z: " << oldRegisters.getStatus('Z')
			<< ", I: " << oldRegisters.getStatus('I')
			<< ", D: " << oldRegisters.getStatus('D')
			<< ", V: " << oldRegisters.getStatus('V')
			<< ", N: " << oldRegisters.getStatus('N');

		std::cout << " | Prev. Cycle #: " << std::dec << this->lastCycleCount << " | (" << this->executedIndex << ")";
	};
	
};

// TODO: Give this a better name.
class CPUDebugger : public _6502_CPU {
public:
	CPUDebugger();
	CPUDebugger(DebugDatabus* databus);
	~CPUDebugger();

	// Note: Currently one cycle = one instruction, but in reality it is different and depends on the specific instruction.
	bool executeCycle() override;

	bool pcAt(uint16_t address);  // Tells you when the PC has reached a certain value; useful for breakpoints.

	void attach(DebugDatabus* databus);

	// Undos an instruction in the stack.
	bool undoInstruction();
	// Returns the last executed instruction
	ExecutedInstruction getLastExecutedInstruction();

	// Gets a fill list, in order, of the executed instructions this CPU has performed.
	// You almost never want a full dump.
	std::vector<ExecutedInstruction> getExecutedInstructions();
	std::vector<ExecutedInstruction> getExecutedInstructions(unsigned int lastN);
	void clearExecutedInstructions();

public:
	// Basic debugging operations which do not affect the internal stack.

	// Returns a range of memory which the CPU sees.
	std::vector<uint8_t> memDump(uint16_t startAddr, uint16_t endAddr);

	// Direct memory operations. Peek = Getter, Poke = Setter, mem = memory. Serve a purely debug role.
	uint8_t memPeek(uint16_t memoryAddress);
	Registers registersPeek();
	void memPoke(uint16_t memoryAddress, uint8_t val);
	void registersPoke(Registers registers);

	// Searches for a memory value and gets the first address which satisifies this condition. Returns true if found, false if not.
	// Range (optional) is inclusive.
	bool memFind(uint8_t value, uint16_t& address, int lowerBound = -1, int upperBound = -1);

	// Sets every address to a certain value.
	void setStdMemValue(uint8_t value);

	// Outputs all the values in the stack.
	std::array<uint8_t, 0x100> dumpStack();
private:
	DebugDatabus* databus;
	
	// NOTE: Might change from a stack to a vector, just because other debugger functions may find that structure more useful.
	// Stack containing the instructions executed in order.
	std::vector<ExecutedInstruction> executedInstructions;
};


// A collection of code which performs a simple test of the above; put inside this function for cleanliness.
void CPUDebuggerTest();