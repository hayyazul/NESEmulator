#include "parseLogData.h"
#include <fstream>
#include <iostream>
#include <sstream>

// TODO: Clean up function.
std::vector<ExecutedOpcodeLogEntry> getTestFileLog(const char* filename) {
	std::ifstream file{filename};
	if (!file.is_open()) {
		std::cout << "Failed to load file" << std::endl;
		return std::vector<ExecutedOpcodeLogEntry>();
	}

	std::string entry;
	std::vector<ExecutedOpcodeLogEntry> log;

	// Variables to hold some data.
	int idx, numOfOperands, cycleCount;
	uint8_t opcode;
	uint8_t operands[2];
	uint8_t ppu[2];
	Registers registers;
	// Each line equals one ExecutedOpcodeLogEntry
	
	while (std::getline(file, entry)) { 
		// Sample line: 2 C5F7 1 86 00 00 | 00 00 00 26 FD 12 0 36
		// There are at least 14 entries in a line.
		std::string entryInLine;
		std::istringstream iss(entry);
		int idxInLine = 0;  // Counts through elements in the line.
		const int numOfEntries = 14;  // This can be increased to 14, depending on the value of the third index.
		while (idxInLine < numOfEntries) {
			std::getline(iss, entryInLine, ' ');
			switch (idxInLine) {
			case(0):
				idx = stoi(entryInLine);
				break;
			case(1):
				registers.PC = stoi(entryInLine, 0, 16);
				break;
			case(2):
				numOfOperands = stoi(entryInLine);
				break;
			case(3):
				opcode = stoi(entryInLine, 0, 16);
				break;
			case(4):
				operands[0] = stoi(entryInLine, 0, 16);
				break;
			case(5):
				operands[1] = stoi(entryInLine, 0, 16);
				break;
			case(6):
				registers.A = stoi(entryInLine, 0, 16);
				break;
			case(7):
				registers.X = stoi(entryInLine, 0, 16);
				break;
			case(8):
				registers.Y = stoi(entryInLine, 0, 16);
				break;
			case(9):
				registers.S = stoi(entryInLine, 0, 16);
				break;
			case(10):
				registers.SP = stoi(entryInLine, 0, 16);
				break;
			case(11):
				cycleCount = stoi(entryInLine);
				break;
			case(12):
				ppu[0] = stoi(entryInLine, 0, 16);
				break;
			case(13):
				ppu[1] = stoi(entryInLine);
				break;
			default:
				break;
			}
			++idxInLine;
		}

		ExecutedOpcodeLogEntry logEntry{ idx, numOfOperands, cycleCount, opcode, operands, ppu, registers };
		log.push_back(logEntry);
	}

	return log;
}