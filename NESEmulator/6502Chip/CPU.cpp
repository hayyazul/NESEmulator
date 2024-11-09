#include "CPU.h"
#include <iostream>
#include <iomanip>

_6502_CPU::_6502_CPU() : databus(nullptr) {
	this->setupInstructionSet();
}

_6502_CPU::_6502_CPU(DataBus* databus) : databus(databus) {
	this->setupInstructionSet();
}

_6502_CPU::~_6502_CPU() {}

bool _6502_CPU::executeCycle() {
	// First check if the number of cycles elapsed corresponds with the number of cycles the instruction takes up. If so, execute the next instruction.
	if (true) { //For now, don't bother. this->opcodeCyclesElapsed == this->currentOpcodeCycleLen) {
		this->opcodeCyclesElapsed = 0;
		uint8_t opcode = this->databus->read(this->registers.PC);  // Get the next opcode.
		// Displays the opcode, its location in memory, the 3 bytes which follow it, the stack sum, and how much the PC was iterated (does not include branch offsets, JMPs, etc. instruction specific PC changes).

		std::map<uint8_t, std::string> opcodesMap = {
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
			{0xFF, "??? ?????"}
		};
		std::cout << opcodesMap[opcode] << " : 0x" << std::hex << std::setfill('0') << std::setw(2) << (int)opcode << " at addr 0x" << std::setw(4) << this->registers.PC << " |";
		for (int i = 1; i <= 3; ++i) {
			std::cout << " 0x" << std::hex << std::setfill('0') << std::setw(2) << (int)this->databus->read(this->registers.PC + i);
			if (i != 3) {
				std::cout << ",";
			}
		}
		// Check if this opcode exists.
		if (!this->instructionSet.contains(opcode)) {
			return false;
		};
		Instruction& instruction = this->instructionSet[opcode];

		this->currentOpcodeCycleLen = instruction.cycleCount;  // Get how many cycles this opcode will be using.
		this->executeOpcode(opcode);
		int s = 0;
		std::array<uint8_t, 0x100> a = this->dumpStack();

		for (auto& b : a) {
			s += b;
		}
		std::cout << " | Stack sum: " << s << " | SP: 0x" << std::setw(2) << (int)registers.SP;
		std::cout << " | PC Iter: " << (int)(instruction.numBytes * !instruction.modifiesPC) << std::endl;
		this->registers.PC += instruction.numBytes * !instruction.modifiesPC;  // Only move the program counter forward if the instruction does not modify the PC.
	}
	++this->opcodeCyclesElapsed;
	++this->totalCyclesElapsed;
	return true;
}

void _6502_CPU::reset() {
	auto a = static_cast<uint16_t>(this->databus->read(RESET_VECTOR_ADDRESS + 1)) << 8;
	auto b = static_cast<uint16_t>(this->databus->read(RESET_VECTOR_ADDRESS));
	registers.PC = static_cast<uint16_t>(this->databus->read(RESET_VECTOR_ADDRESS)) + (static_cast<uint16_t>(this->databus->read(RESET_VECTOR_ADDRESS + 1)) << 8);
	registers.SP -= 3;
	registers.setStatus('I', true);
}

void _6502_CPU::powerOn() {
	this->reset();
	this->registers.setStatus('C', 0);
	this->registers.setStatus('Z', 0);
	this->registers.setStatus('D', 0);
	this->registers.setStatus('V', 0);
	this->registers.setStatus('N', 0);
}

void _6502_CPU::executeOpcode(uint8_t opcode) {
	Instruction& instruction = this->instructionSet[opcode];  // Here for debugging purposes.
	instruction.performOperation(this->registers, *this->databus);
}

uint8_t _6502_CPU::memPeek(uint16_t memoryAddress) {
	return this->databus->read(memoryAddress);
}

Registers _6502_CPU::registersPeek() {
	return this->registers;
}

void _6502_CPU::memPoke(uint16_t memoryAddress, uint8_t val) {
	this->databus->write(memoryAddress, val);
}

void _6502_CPU::registersPoke(Registers registers) {
	this->registers = registers;
}

// Sets address to the first address which equals the given value if it is found; otherwise it remains unchanged.
bool _6502_CPU::memFind(uint8_t value, uint16_t& address, int lowerBound, int upperBound) {
	uint16_t startAddr = lowerBound == -1 ? 0 : (uint16_t)lowerBound;
	uint16_t endAddr = upperBound == -1 ? 0xffff : (uint16_t)upperBound;
	for (uint16_t i = startAddr; i <= endAddr; ++i) {
		if (this->databus->read(i) == value) {
			address = i;
			return true;
		};

		if (i == endAddr) {
			break;
		}
	}
	return false;
}

std::array<uint8_t, 0x100> _6502_CPU::dumpStack() {
	std::array<uint8_t, 0x100> stack;
	for (uint8_t i = 0x0; i <= 0xff; ++i) {
		stack[i] = this->databus->read(i + STACK_END_ADDR);
		if (i == 0xff) {
			break;
		}
	}
	return stack;
}

void _6502_CPU::setupInstructionSet() {
	// bne, DEX, EOR,  LDA, LDX, LDY, PHA, PHP, SBC, SEC, SEI, STA, STX, STY, ALL Ts
	
	// ADC (Add with Carry)
	this->instructionSet[0x69] = Instruction(ops::ADC, addrModes::immediate, 2, 2);
	this->instructionSet[0x65] = Instruction(ops::ADC, addrModes::zeropage, 2, 3);
	this->instructionSet[0x75] = Instruction(ops::ADC, addrModes::zeropageX, 2, 4);
	this->instructionSet[0x6d] = Instruction(ops::ADC, addrModes::absolute, 3, 4);
	this->instructionSet[0x7d] = Instruction(ops::ADC, addrModes::absoluteX, 3, 4);
	this->instructionSet[0x79] = Instruction(ops::ADC, addrModes::absoluteY, 3, 4);
	this->instructionSet[0x61] = Instruction(ops::ADC, addrModes::indirectX, 2, 6);
	this->instructionSet[0x71] = Instruction(ops::ADC, addrModes::indirectY, 2, 5);

	// AND (Logical AND)
	this->instructionSet[0x29] = Instruction(ops::AND, addrModes::immediate, 2, 2);
	this->instructionSet[0x25] = Instruction(ops::AND, addrModes::zeropage, 2, 3);
	this->instructionSet[0x35] = Instruction(ops::AND, addrModes::zeropageX, 2, 4);
	this->instructionSet[0x2d] = Instruction(ops::AND, addrModes::absolute, 3, 4);
	this->instructionSet[0x3d] = Instruction(ops::AND, addrModes::absoluteX, 3, 4);
	this->instructionSet[0x39] = Instruction(ops::AND, addrModes::absoluteY, 3, 4);
	this->instructionSet[0x21] = Instruction(ops::AND, addrModes::indirectX, 2, 6);
	this->instructionSet[0x31] = Instruction(ops::AND, addrModes::indirectY, 2, 5);

	// ASL (Arithmetic Shift Left)
	this->instructionSet[0x0a] = Instruction((RegOp)ops::ASL, addrModes::accumulator, 1, 2);
	this->instructionSet[0x06] = Instruction((MemOp)ops::ASL, addrModes::zeropage, 2, 5);
	this->instructionSet[0x16] = Instruction((MemOp)ops::ASL, addrModes::zeropageX, 2, 6);
	this->instructionSet[0x0e] = Instruction((MemOp)ops::ASL, addrModes::absolute, 3, 6);
	this->instructionSet[0x1e] = Instruction((MemOp)ops::ASL, addrModes::absoluteX, 3, 7);

	// Branch Operations (BCC, BCS, BEQ, BMI, BPL, BVC, BVS)
	this->instructionSet[0x90] = Instruction(ops::BCC, addrModes::relative, 2, 2, true);
	this->instructionSet[0xb0] = Instruction(ops::BCS, addrModes::relative, 2, 2, true);
	this->instructionSet[0xf0] = Instruction(ops::BEQ, addrModes::relative, 2, 2, true);
	this->instructionSet[0x30] = Instruction(ops::BMI, addrModes::relative, 2, 2, true);
	this->instructionSet[0x10] = Instruction(ops::BPL, addrModes::relative, 2, 2, true);
	this->instructionSet[0x50] = Instruction(ops::BVC, addrModes::relative, 2, 2, true);
	this->instructionSet[0x70] = Instruction(ops::BVS, addrModes::relative, 2, 2, true);

	// Bit Test (BIT)
	this->instructionSet[0x24] = Instruction(ops::BIT, addrModes::zeropage, 2, 3);
	this->instructionSet[0x2c] = Instruction(ops::BIT, addrModes::absolute, 3, 4);

	// Clear and Set Flag Instructions (CLC, CLI, CLV)
	this->instructionSet[0x18] = Instruction(ops::CLC, addrModes::implicit, 1, 2);
	this->instructionSet[0x58] = Instruction(ops::CLI, addrModes::implicit, 1, 2);
	this->instructionSet[0xb8] = Instruction(ops::CLV, addrModes::implicit, 1, 2);
	this->instructionSet[0xd8] = Instruction(ops::CLD, addrModes::implicit, 1, 2);

	// Compare (CMP, CPX, CPY)
	this->instructionSet[0xc9] = Instruction(ops::CMP, addrModes::immediate, 2, 2);
	this->instructionSet[0xc5] = Instruction(ops::CMP, addrModes::zeropage, 2, 3);
	this->instructionSet[0xd5] = Instruction(ops::CMP, addrModes::zeropageX, 2, 4);
	this->instructionSet[0xcd] = Instruction(ops::CMP, addrModes::absolute, 3, 4);
	this->instructionSet[0xdd] = Instruction(ops::CMP, addrModes::absoluteX, 3, 4);
	this->instructionSet[0xd9] = Instruction(ops::CMP, addrModes::absoluteY, 3, 4);
	this->instructionSet[0xc1] = Instruction(ops::CMP, addrModes::indirectX, 2, 6);
	this->instructionSet[0xd1] = Instruction(ops::CMP, addrModes::indirectY, 2, 5);

	this->instructionSet[0xe0] = Instruction(ops::CPX, addrModes::immediate, 2, 2);
	this->instructionSet[0xe4] = Instruction(ops::CPX, addrModes::zeropage, 2, 3);
	this->instructionSet[0xec] = Instruction(ops::CPX, addrModes::absolute, 3, 4);

	this->instructionSet[0xc0] = Instruction(ops::CPY, addrModes::immediate, 2, 2);
	this->instructionSet[0xc4] = Instruction(ops::CPY, addrModes::zeropage, 2, 3);
	this->instructionSet[0xcc] = Instruction(ops::CPY, addrModes::absolute, 3, 4);

	// Jump (JMP)
	this->instructionSet[0x4c] = Instruction(ops::JMP, addrModes::absolute, 3, 3, true);
	this->instructionSet[0x6c] = Instruction(ops::JMP, addrModes::indirect, 3, 5, true);

	// Jump to Subroutine and Return from Subroutine (JSR, RTS)
	this->instructionSet[0x20] = Instruction(ops::JSR, addrModes::absolute, 3, 6, true);
	this->instructionSet[0x60] = Instruction(ops::RTS, addrModes::implicit, 1, 6, true);

	// Increment (INC)
	this->instructionSet[0xe6] = Instruction(ops::INC, addrModes::zeropage, 2, 5);
	this->instructionSet[0xf6] = Instruction(ops::INC, addrModes::zeropageX, 2, 6);
	this->instructionSet[0xee] = Instruction(ops::INC, addrModes::absolute, 3, 6);
	this->instructionSet[0xfe] = Instruction(ops::INC, addrModes::absoluteX, 3, 7);

	// Decrement (DEC)
	this->instructionSet[0xc6] = Instruction(ops::DEC, addrModes::zeropage, 2, 5);
	this->instructionSet[0xd6] = Instruction(ops::DEC, addrModes::zeropageX, 2, 6);
	this->instructionSet[0xce] = Instruction(ops::DEC, addrModes::absolute, 3, 6);
	this->instructionSet[0xde] = Instruction(ops::DEC, addrModes::absoluteX, 3, 7);

	// Increment and Decrement Index Registers (INX, INY, DEY)
	this->instructionSet[0xe8] = Instruction(ops::INX, addrModes::implicit, 1, 2);
	this->instructionSet[0xc8] = Instruction(ops::INY, addrModes::implicit, 1, 2);
	this->instructionSet[0x88] = Instruction(ops::DEY, addrModes::implicit, 1, 2);

	// Logical Shift Right (LSR)
	this->instructionSet[0x4a] = Instruction((RegOp)ops::LSR, addrModes::accumulator, 1, 2);
	this->instructionSet[0x46] = Instruction((MemOp)ops::LSR, addrModes::zeropage, 2, 5);
	this->instructionSet[0x56] = Instruction((MemOp)ops::LSR, addrModes::zeropageX, 2, 6);
	this->instructionSet[0x4e] = Instruction((MemOp)ops::LSR, addrModes::absolute, 3, 6);
	this->instructionSet[0x5e] = Instruction((MemOp)ops::LSR, addrModes::absoluteX, 3, 7);

	// NOP (No Operation)
	this->instructionSet[0xEA] = Instruction((RegOp)ops::NOP, addrModes::implicit, 1, 2);  // Reg or Mem op; doesn't matter which.

	// ORA (Logical Inclusive OR)
	this->instructionSet[0x09] = Instruction(ops::ORA, addrModes::immediate, 2, 2);
	this->instructionSet[0x05] = Instruction(ops::ORA, addrModes::zeropage, 2, 3);
	this->instructionSet[0x15] = Instruction(ops::ORA, addrModes::zeropageX, 2, 4);
	this->instructionSet[0x0D] = Instruction(ops::ORA, addrModes::absolute, 3, 4);
	this->instructionSet[0x1D] = Instruction(ops::ORA, addrModes::absoluteX, 3, 4);
	this->instructionSet[0x19] = Instruction(ops::ORA, addrModes::absoluteY, 3, 4);
	this->instructionSet[0x01] = Instruction(ops::ORA, addrModes::indirectX, 2, 6);
	this->instructionSet[0x11] = Instruction(ops::ORA, addrModes::indirectY, 2, 5);

	// PLA (Pull Accumulator from Stack)
	this->instructionSet[0x68] = Instruction(ops::PLA, addrModes::implicit, 1, 4);

	// PLP (Pull Processor Status from Stack)
	this->instructionSet[0x28] = Instruction(ops::PLP, addrModes::implicit, 1, 4);

	// RTI (Return from Interrupt)
	this->instructionSet[0x40] = Instruction(ops::RTI, addrModes::implicit, 1, 6);

	// ROR (Rotate Right)
	this->instructionSet[0x6a] = Instruction((RegOp)ops::ROR, addrModes::accumulator, 1, 2);
	this->instructionSet[0x66] = Instruction((MemOp)ops::ROR, addrModes::zeropage, 2, 5);
	this->instructionSet[0x76] = Instruction((MemOp)ops::ROR, addrModes::zeropageX, 2, 6);
	this->instructionSet[0x6e] = Instruction((MemOp)ops::ROR, addrModes::absolute, 3, 6);
	this->instructionSet[0x7e] = Instruction((MemOp)ops::ROR, addrModes::absoluteX, 3, 7);

	// ROL (Rotate Left)
	this->instructionSet[0x2a] = Instruction((RegOp)ops::ROL, addrModes::accumulator, 1, 2);
	this->instructionSet[0x26] = Instruction((MemOp)ops::ROL, addrModes::zeropage, 2, 5);
	this->instructionSet[0x36] = Instruction((MemOp)ops::ROL, addrModes::zeropageX, 2, 6);
	this->instructionSet[0x2e] = Instruction((MemOp)ops::ROL, addrModes::absolute, 3, 6);
	this->instructionSet[0x3e] = Instruction((MemOp)ops::ROL, addrModes::absoluteX, 3, 7);

	// BNE (Branch if Not Equal)
	this->instructionSet[0xd0] = Instruction(ops::BNE, addrModes::relative, 2, 2, true);

	// DEX (Decrement X)
	this->instructionSet[0xca] = Instruction(ops::DEX, addrModes::implicit, 1, 2);

	// EOR (Exclusive OR)
	this->instructionSet[0x49] = Instruction(ops::EOR, addrModes::immediate, 2, 2);
	this->instructionSet[0x45] = Instruction(ops::EOR, addrModes::zeropage, 2, 3);
	this->instructionSet[0x55] = Instruction(ops::EOR, addrModes::zeropageX, 2, 4);
	this->instructionSet[0x4d] = Instruction(ops::EOR, addrModes::absolute, 3, 4);
	this->instructionSet[0x5d] = Instruction(ops::EOR, addrModes::absoluteX, 3, 4);
	this->instructionSet[0x59] = Instruction(ops::EOR, addrModes::absoluteY, 3, 4);
	this->instructionSet[0x41] = Instruction(ops::EOR, addrModes::indirectX, 2, 6);
	this->instructionSet[0x51] = Instruction(ops::EOR, addrModes::indirectY, 2, 5);

	// LDA (Load Accumulator)
	this->instructionSet[0xa9] = Instruction(ops::LDA, addrModes::immediate, 2, 2);
	this->instructionSet[0xa5] = Instruction(ops::LDA, addrModes::zeropage, 2, 3);
	this->instructionSet[0xb5] = Instruction(ops::LDA, addrModes::zeropageX, 2, 4);
	this->instructionSet[0xad] = Instruction(ops::LDA, addrModes::absolute, 3, 4);
	this->instructionSet[0xbd] = Instruction(ops::LDA, addrModes::absoluteX, 3, 4);
	this->instructionSet[0xb9] = Instruction(ops::LDA, addrModes::absoluteY, 3, 4);
	this->instructionSet[0xa1] = Instruction(ops::LDA, addrModes::indirectX, 2, 6);
	this->instructionSet[0xb1] = Instruction(ops::LDA, addrModes::indirectY, 2, 5);

	// LDX (Load X)
	this->instructionSet[0xa2] = Instruction(ops::LDX, addrModes::immediate, 2, 2);
	this->instructionSet[0xa6] = Instruction(ops::LDX, addrModes::zeropage, 2, 3);
	this->instructionSet[0xb6] = Instruction(ops::LDX, addrModes::zeropageY, 2, 4);
	this->instructionSet[0xae] = Instruction(ops::LDX, addrModes::absolute, 3, 4);
	this->instructionSet[0xbe] = Instruction(ops::LDX, addrModes::absoluteY, 3, 4);

	// LDY (Load Y)
	this->instructionSet[0xa0] = Instruction(ops::LDY, addrModes::immediate, 2, 2);
	this->instructionSet[0xa4] = Instruction(ops::LDY, addrModes::zeropage, 2, 3);
	this->instructionSet[0xb4] = Instruction(ops::LDY, addrModes::zeropageX, 2, 4);
	this->instructionSet[0xac] = Instruction(ops::LDY, addrModes::absolute, 3, 4);
	this->instructionSet[0xbc] = Instruction(ops::LDY, addrModes::absoluteX, 3, 4);

	// PHA (Push Accumulator)
	this->instructionSet[0x48] = Instruction(ops::PHA, addrModes::implicit, 1, 3);

	// PHP (Push Processor Status)
	this->instructionSet[0x08] = Instruction(ops::PHP, addrModes::implicit, 1, 3);

	// SBC (Subtract with Carry)
	this->instructionSet[0xe9] = Instruction(ops::SBC, addrModes::immediate, 2, 2);
	this->instructionSet[0xe5] = Instruction(ops::SBC, addrModes::zeropage, 2, 3);
	this->instructionSet[0xf5] = Instruction(ops::SBC, addrModes::zeropageX, 2, 4);
	this->instructionSet[0xed] = Instruction(ops::SBC, addrModes::absolute, 3, 4);
	this->instructionSet[0xfd] = Instruction(ops::SBC, addrModes::absoluteX, 3, 4);
	this->instructionSet[0xf9] = Instruction(ops::SBC, addrModes::absoluteY, 3, 4);
	this->instructionSet[0xe1] = Instruction(ops::SBC, addrModes::indirectX, 2, 6);
	this->instructionSet[0xf1] = Instruction(ops::SBC, addrModes::indirectY, 2, 5);

	// SEC (Set Carry Flag)
	this->instructionSet[0x38] = Instruction(ops::SEC, addrModes::implicit, 1, 2);

	// SEI (Set Interrupt Disable)
	this->instructionSet[0x78] = Instruction(ops::SEI, addrModes::implicit, 1, 2);

	// SED (Set Decimal Flag)
	this->instructionSet[0xf8] = Instruction(ops::SED, addrModes::implicit, 1, 2);

	// STA (Store Accumulator)
	this->instructionSet[0x85] = Instruction(ops::STA, addrModes::zeropage, 2, 3);
	this->instructionSet[0x95] = Instruction(ops::STA, addrModes::zeropageX, 2, 4);
	this->instructionSet[0x8d] = Instruction(ops::STA, addrModes::absolute, 2, 4);
	this->instructionSet[0x9d] = Instruction(ops::STA, addrModes::absoluteX, 3, 5);
	this->instructionSet[0x99] = Instruction(ops::STA, addrModes::absoluteY, 3, 5);
	this->instructionSet[0x81] = Instruction(ops::STA, addrModes::indirectX, 2, 6);
	this->instructionSet[0x91] = Instruction(ops::STA, addrModes::indirectY, 2, 6);

	// STX (Store X)
	this->instructionSet[0x86] = Instruction(ops::STX, addrModes::zeropage, 2, 3);
	this->instructionSet[0x96] = Instruction(ops::STX, addrModes::zeropageY, 2, 4);
	this->instructionSet[0x8e] = Instruction(ops::STX, addrModes::absolute, 3, 4);

	// STY (Store Y)
	this->instructionSet[0x84] = Instruction(ops::STY, addrModes::zeropage, 2, 3);
	this->instructionSet[0x94] = Instruction(ops::STY, addrModes::zeropageX, 2, 4);
	this->instructionSet[0x8c] = Instruction(ops::STY, addrModes::absolute, 2, 4);

	// TXA (Transfer X to A)
	this->instructionSet[0x8a] = Instruction(ops::TXA, addrModes::implicit, 1, 2);

	// TXS (Transfer X to Stack Pointer)
	this->instructionSet[0x9a] = Instruction(ops::TXS, addrModes::implicit, 1, 2);

	// TYA (Transfer Y to A)
	this->instructionSet[0x98] = Instruction(ops::TYA, addrModes::implicit, 1, 2);

	// TAY (Transfer A to Y)
	this->instructionSet[0xa8] = Instruction(ops::TAY, addrModes::implicit, 1, 2);

	// TAX (Transfer A to X)
	this->instructionSet[0xaa] = Instruction(ops::TAX, addrModes::implicit, 1, 2);

	// TSX (Transfer Stack Pointer to X)
	this->instructionSet[0xba] = Instruction(ops::TSX, addrModes::implicit, 1, 2);
}