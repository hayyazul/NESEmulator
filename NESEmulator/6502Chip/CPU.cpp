#include "CPU.h"

_6502_CPU::_6502_CPU() : databus(nullptr) {
	this->setupInstructionSet();
}

_6502_CPU::_6502_CPU(DataBus* databus) : databus(databus) {
	this->setupInstructionSet();
}

_6502_CPU::~_6502_CPU() {}

void _6502_CPU::executeCycle() {
	// First check if the number of cycles elapsed corresponds with the number of cycles the instruction takes up. If so, execute the next instruction.
	if (this->opcodeCyclesElapsed == this->currentOpcodeCycleLen) {
		this->currentOpcodeCycleLen = 0;  // Reset the opcode counter.
		uint8_t opcode = this->databus->read(this->registers.PC);  // Get the next opcode.
		this->currentOpcodeCycleLen = this->instructionSet[opcode].cycleCount;  // Get how many cycles this opcode will be using.

		this->executeOpcode(opcode);
		auto debugVar1 = this->instructionSet[opcode].numBytes;
		auto debugVar2 = !this->instructionSet[opcode].modifiesPC;
		this->registers.PC += this->instructionSet[opcode].numBytes * !this->instructionSet[opcode].modifiesPC;  // Only move the program counter forward if the instruction does not modify the PC.
	}
	++this->opcodeCyclesElapsed;
	++this->totalCyclesElapsed;
}

void _6502_CPU::reset() {
	registers.PC = static_cast<uint16_t>(this->databus->read(RESET_VECTOR_ADDRESS)) + static_cast<uint16_t>(this->databus->read(RESET_VECTOR_ADDRESS + 1)) << 8;
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
	this->instructionSet[opcode].performOperation(this->registers, *this->databus);
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

void _6502_CPU::setupInstructionSet() {
	// bne, DEX, EOR,  LDA, LDX, LDY, PHA, PHP, SBC, SEC, SEI, STA, STX, STY, ALL Ts
	
	// ADC (Add with Carry)
	this->instructionSet[0x69] = Instruction(ops::ADC, addrModes::immediate, 2, 2);
	this->instructionSet[0x65] = Instruction(ops::ADC, addrModes::zeropage, 2, 3);
	this->instructionSet[0x75] = Instruction(ops::ADC, addrModes::zeropageX, 2, 4);
	this->instructionSet[0x6d] = Instruction(ops::ADC, addrModes::absolute, 2, 4);
	this->instructionSet[0x7d] = Instruction(ops::ADC, addrModes::absoluteX, 2, 4);
	this->instructionSet[0x79] = Instruction(ops::ADC, addrModes::absoluteY, 2, 4);
	this->instructionSet[0x61] = Instruction(ops::ADC, addrModes::indirectX, 2, 6);
	this->instructionSet[0x71] = Instruction(ops::ADC, addrModes::indirectY, 2, 5);

	// AND (Logical AND)
	this->instructionSet[0x29] = Instruction(ops::AND, addrModes::immediate, 2, 2);
	this->instructionSet[0x25] = Instruction(ops::AND, addrModes::zeropage, 2, 3);
	this->instructionSet[0x35] = Instruction(ops::AND, addrModes::zeropageX, 2, 4);
	this->instructionSet[0x2d] = Instruction(ops::AND, addrModes::absolute, 2, 4);
	this->instructionSet[0x3d] = Instruction(ops::AND, addrModes::absoluteX, 2, 4);
	this->instructionSet[0x39] = Instruction(ops::AND, addrModes::absoluteY, 2, 4);
	this->instructionSet[0x21] = Instruction(ops::AND, addrModes::indirectX, 2, 6);
	this->instructionSet[0x31] = Instruction(ops::AND, addrModes::indirectY, 2, 5);

	// ASL (Arithmetic Shift Left)
	this->instructionSet[0x0a] = Instruction((RegOp)ops::ASL, addrModes::accumulator, 1, 2);
	this->instructionSet[0x06] = Instruction((MemOp)ops::ASL, addrModes::zeropage, 2, 5);
	this->instructionSet[0x16] = Instruction((MemOp)ops::ASL, addrModes::zeropageX, 2, 6);
	this->instructionSet[0x0e] = Instruction((MemOp)ops::ASL, addrModes::absolute, 2, 6);
	this->instructionSet[0x1e] = Instruction((MemOp)ops::ASL, addrModes::absoluteX, 2, 7);

	// Branch Operations (BCC, BCS, BEQ, BMI, BPL, BVC, BVS)
	this->instructionSet[0x90] = Instruction(ops::BCC, addrModes::relative, 2, 2);
	this->instructionSet[0xb0] = Instruction(ops::BCS, addrModes::relative, 2, 2);
	this->instructionSet[0xf0] = Instruction(ops::BEQ, addrModes::relative, 2, 2);
	this->instructionSet[0x30] = Instruction(ops::BMI, addrModes::relative, 2, 2);
	this->instructionSet[0x10] = Instruction(ops::BPL, addrModes::relative, 2, 2);
	this->instructionSet[0x50] = Instruction(ops::BVC, addrModes::relative, 2, 2);
	this->instructionSet[0x70] = Instruction(ops::BVS, addrModes::relative, 2, 2);

	// Bit Test (BIT)
	this->instructionSet[0x24] = Instruction(ops::BIT, addrModes::zeropage, 2, 3);
	this->instructionSet[0x2c] = Instruction(ops::BIT, addrModes::absolute, 2, 4);

	// Clear and Set Flag Instructions (CLC, CLI, CLV)
	this->instructionSet[0x18] = Instruction(ops::CLC, addrModes::implicit, 1, 2);
	this->instructionSet[0x58] = Instruction(ops::CLI, addrModes::implicit, 1, 2);
	this->instructionSet[0xb8] = Instruction(ops::CLV, addrModes::implicit, 1, 2);

	// Compare (CMP, CPX, CPY)
	this->instructionSet[0xc9] = Instruction(ops::CMP, addrModes::immediate, 2, 2);
	this->instructionSet[0xc5] = Instruction(ops::CMP, addrModes::zeropage, 2, 3);
	this->instructionSet[0xd5] = Instruction(ops::CMP, addrModes::zeropageX, 2, 4);
	this->instructionSet[0x8d] = Instruction(ops::CMP, addrModes::absolute, 2, 4);
	this->instructionSet[0x9d] = Instruction(ops::CMP, addrModes::absoluteX, 2, 4);
	this->instructionSet[0x99] = Instruction(ops::CMP, addrModes::absoluteY, 2, 4);
	this->instructionSet[0x61] = Instruction(ops::CMP, addrModes::indirectX, 2, 6);
	this->instructionSet[0x71] = Instruction(ops::CMP, addrModes::indirectY, 2, 5);

	this->instructionSet[0xe0] = Instruction(ops::CPX, addrModes::immediate, 2, 2);
	this->instructionSet[0xe4] = Instruction(ops::CPX, addrModes::zeropage, 2, 3);
	this->instructionSet[0xec] = Instruction(ops::CPX, addrModes::absolute, 2, 4);

	this->instructionSet[0xc0] = Instruction(ops::CPY, addrModes::immediate, 2, 2);
	this->instructionSet[0xc4] = Instruction(ops::CPY, addrModes::zeropage, 2, 3);
	this->instructionSet[0xcc] = Instruction(ops::CPY, addrModes::absolute, 2, 4);

	// Jump (JMP)
	this->instructionSet[0x4c] = Instruction(ops::JMP, addrModes::absolute, 3, 3);
	this->instructionSet[0x6c] = Instruction(ops::JMP, addrModes::indirect, 3, 5);

	// Jump to Subroutine and Return from Subroutine (JSR, RTS)
	this->instructionSet[0x20] = Instruction(ops::JSR, addrModes::absolute, 3, 6);
	this->instructionSet[0x60] = Instruction(ops::RTS, addrModes::implicit, 1, 6);

	// Increment (INC)
	this->instructionSet[0xe6] = Instruction(ops::INC, addrModes::zeropage, 2, 5);
	this->instructionSet[0xf6] = Instruction(ops::INC, addrModes::zeropageX, 2, 6);
	this->instructionSet[0xee] = Instruction(ops::INC, addrModes::absolute, 2, 6);
	this->instructionSet[0xfe] = Instruction(ops::INC, addrModes::absoluteX, 2, 7);

	// Decrement (DEC)
	this->instructionSet[0xc6] = Instruction(ops::DEC, addrModes::zeropage, 2, 5);
	this->instructionSet[0xd6] = Instruction(ops::DEC, addrModes::zeropageX, 2, 6);
	this->instructionSet[0xce] = Instruction(ops::DEC, addrModes::absolute, 2, 6);
	this->instructionSet[0xde] = Instruction(ops::DEC, addrModes::absoluteX, 2, 7);

	// Increment and Decrement Index Registers (INX, INY, DEY)
	this->instructionSet[0xe8] = Instruction(ops::INX, addrModes::implicit, 1, 2);
	this->instructionSet[0xc8] = Instruction(ops::INY, addrModes::implicit, 1, 2);
	this->instructionSet[0x88] = Instruction(ops::DEY, addrModes::implicit, 1, 2);

	// Logical Shift Right (LSR)
	this->instructionSet[0x4a] = Instruction((RegOp)ops::LSR, addrModes::accumulator, 1, 2);
	this->instructionSet[0x46] = Instruction((MemOp)ops::LSR, addrModes::zeropage, 2, 5);
	this->instructionSet[0x56] = Instruction((MemOp)ops::LSR, addrModes::zeropageX, 2, 6);
	this->instructionSet[0x4e] = Instruction((MemOp)ops::LSR, addrModes::absolute, 2, 6);
	this->instructionSet[0x5e] = Instruction((MemOp)ops::LSR, addrModes::absoluteX, 2, 7);

	// NOP (No Operation)
	this->instructionSet[0xEA] = Instruction((RegOp)ops::NOP, addrModes::implicit, 1, 2);  // Reg or Mem op; doesn't matter which.

	// ORA (Logical Inclusive OR)
	this->instructionSet[0x09] = Instruction(ops::ORA, addrModes::immediate, 2, 2);
	this->instructionSet[0x05] = Instruction(ops::ORA, addrModes::zeropage, 2, 3);
	this->instructionSet[0x15] = Instruction(ops::ORA, addrModes::zeropageX, 2, 4);
	this->instructionSet[0x0D] = Instruction(ops::ORA, addrModes::absolute, 2, 4);
	this->instructionSet[0x1D] = Instruction(ops::ORA, addrModes::absoluteX, 2, 4);
	this->instructionSet[0x19] = Instruction(ops::ORA, addrModes::absoluteY, 2, 4);
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
	this->instructionSet[0x6e] = Instruction((MemOp)ops::ROR, addrModes::absolute, 2, 6);
	this->instructionSet[0x7e] = Instruction((MemOp)ops::ROR, addrModes::absoluteX, 2, 7);

	// ROL (Rotate Left)
	this->instructionSet[0x2a] = Instruction((RegOp)ops::ROL, addrModes::accumulator, 1, 2);
	this->instructionSet[0x26] = Instruction((MemOp)ops::ROL, addrModes::zeropage, 2, 5);
	this->instructionSet[0x36] = Instruction((MemOp)ops::ROL, addrModes::zeropageX, 2, 6);
	this->instructionSet[0x2e] = Instruction((MemOp)ops::ROL, addrModes::absolute, 2, 6);
	this->instructionSet[0x3e] = Instruction((MemOp)ops::ROL, addrModes::absoluteX, 2, 7);

	// BNE (Branch if Not Equal)
	this->instructionSet[0xd0] = Instruction(ops::BNE, addrModes::relative, 2, 2);

	// DEX (Decrement X)
	this->instructionSet[0xca] = Instruction(ops::DEX, addrModes::implicit, 1, 2);

	// EOR (Exclusive OR)
	this->instructionSet[0x49] = Instruction(ops::EOR, addrModes::immediate, 2, 2);
	this->instructionSet[0x45] = Instruction(ops::EOR, addrModes::zeropage, 2, 3);
	this->instructionSet[0x55] = Instruction(ops::EOR, addrModes::zeropageX, 2, 4);
	this->instructionSet[0x4d] = Instruction(ops::EOR, addrModes::absolute, 2, 4);
	this->instructionSet[0x5d] = Instruction(ops::EOR, addrModes::absoluteX, 2, 4);
	this->instructionSet[0x59] = Instruction(ops::EOR, addrModes::absoluteY, 2, 4);
	this->instructionSet[0x41] = Instruction(ops::EOR, addrModes::indirectX, 2, 6);
	this->instructionSet[0x51] = Instruction(ops::EOR, addrModes::indirectY, 2, 5);

	// LDA (Load Accumulator)
	this->instructionSet[0xa9] = Instruction(ops::LDA, addrModes::immediate, 2, 2);
	this->instructionSet[0xa5] = Instruction(ops::LDA, addrModes::zeropage, 2, 3);
	this->instructionSet[0xb5] = Instruction(ops::LDA, addrModes::zeropageX, 2, 4);
	this->instructionSet[0xad] = Instruction(ops::LDA, addrModes::absolute, 2, 4);
	this->instructionSet[0xbd] = Instruction(ops::LDA, addrModes::absoluteX, 2, 4);
	this->instructionSet[0xb9] = Instruction(ops::LDA, addrModes::absoluteY, 2, 4);
	this->instructionSet[0xa1] = Instruction(ops::LDA, addrModes::indirectX, 2, 6);
	this->instructionSet[0xb1] = Instruction(ops::LDA, addrModes::indirectY, 2, 5);

	// LDX (Load X)
	this->instructionSet[0xa2] = Instruction(ops::LDX, addrModes::immediate, 2, 2);
	this->instructionSet[0xa6] = Instruction(ops::LDX, addrModes::zeropage, 2, 3);
	this->instructionSet[0xb6] = Instruction(ops::LDX, addrModes::zeropageY, 2, 4);
	this->instructionSet[0xae] = Instruction(ops::LDX, addrModes::absolute, 2, 4);
	this->instructionSet[0xbe] = Instruction(ops::LDX, addrModes::absoluteY, 2, 4);

	// LDY (Load Y)
	this->instructionSet[0xa0] = Instruction(ops::LDY, addrModes::immediate, 2, 2);
	this->instructionSet[0xa4] = Instruction(ops::LDY, addrModes::zeropage, 2, 3);
	this->instructionSet[0xb4] = Instruction(ops::LDY, addrModes::zeropageX, 2, 4);
	this->instructionSet[0xac] = Instruction(ops::LDY, addrModes::absolute, 2, 4);
	this->instructionSet[0xbc] = Instruction(ops::LDY, addrModes::absoluteX, 2, 4);

	// PHA (Push Accumulator)
	this->instructionSet[0x48] = Instruction(ops::PHA, addrModes::implicit, 1, 3);

	// PHP (Push Processor Status)
	this->instructionSet[0x08] = Instruction(ops::PHP, addrModes::implicit, 1, 3);

	// SBC (Subtract with Carry)
	this->instructionSet[0xe9] = Instruction(ops::SBC, addrModes::immediate, 2, 2);
	this->instructionSet[0xe5] = Instruction(ops::SBC, addrModes::zeropage, 2, 3);
	this->instructionSet[0xf5] = Instruction(ops::SBC, addrModes::zeropageX, 2, 4);
	this->instructionSet[0xed] = Instruction(ops::SBC, addrModes::absolute, 2, 4);
	this->instructionSet[0xfd] = Instruction(ops::SBC, addrModes::absoluteX, 2, 4);
	this->instructionSet[0xf9] = Instruction(ops::SBC, addrModes::absoluteY, 2, 4);
	this->instructionSet[0xe1] = Instruction(ops::SBC, addrModes::indirectX, 2, 6);
	this->instructionSet[0xf1] = Instruction(ops::SBC, addrModes::indirectY, 2, 5);

	// SEC (Set Carry Flag)
	this->instructionSet[0x38] = Instruction(ops::SEC, addrModes::implicit, 1, 2);

	// SEI (Set Interrupt Disable)
	this->instructionSet[0x78] = Instruction(ops::SEI, addrModes::implicit, 1, 2);

	// STA (Store Accumulator)
	this->instructionSet[0x85] = Instruction(ops::STA, addrModes::zeropage, 2, 3);
	this->instructionSet[0x95] = Instruction(ops::STA, addrModes::zeropageX, 2, 4);
	this->instructionSet[0x8d] = Instruction(ops::STA, addrModes::absolute, 2, 4);
	this->instructionSet[0x9d] = Instruction(ops::STA, addrModes::absoluteX, 2, 5);
	this->instructionSet[0x99] = Instruction(ops::STA, addrModes::absoluteY, 2, 5);
	this->instructionSet[0x81] = Instruction(ops::STA, addrModes::indirectX, 2, 6);
	this->instructionSet[0x91] = Instruction(ops::STA, addrModes::indirectY, 2, 6);

	// STX (Store X)
	this->instructionSet[0x86] = Instruction(ops::STX, addrModes::zeropage, 2, 3);
	this->instructionSet[0x96] = Instruction(ops::STX, addrModes::zeropageY, 2, 4);
	this->instructionSet[0x8e] = Instruction(ops::STX, addrModes::absolute, 2, 4);

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
	this->instructionSet[0x8d] = Instruction(ops::TAX, addrModes::implicit, 1, 2);

	// TSX (Transfer Stack Pointer to X)
	this->instructionSet[0x9e] = Instruction(ops::TSX, addrModes::implicit, 1, 2);
}

// Cycle operations