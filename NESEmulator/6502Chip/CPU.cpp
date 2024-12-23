#include "CPU.h"
#include <iostream>
#include <iomanip>

_6502_CPU::_6502_CPU() : databus(nullptr), interruptRequested(false), performInterrupt(false), nmiRequested(false) {
	this->setupInstructionSet();
}

_6502_CPU::_6502_CPU(DataBus* databus) : databus(databus), interruptRequested(false), performInterrupt(false), nmiRequested(false) {
	this->setupInstructionSet();
}

_6502_CPU::~_6502_CPU() {}

void _6502_CPU::attach(DataBus* databus) {
	this->databus = databus;
}

CPUCycleOutcomes _6502_CPU::executeCycle() {
	// First check if the number of cycles elapsed corresponds with the number of cycles the instruction takes up. If so, execute the next instruction.
	CPUCycleOutcomes outcome = PASS;

	if (this->opcodeCyclesElapsed == this->currentOpcodeCycleLen) {

		if (this->nmiRequested) {
			this->performNMIActions();
		} else if (this->performInterrupt) {
			this->performInterruptActions();
		}

		outcome = INSTRUCTION_EXECUTED;
		this->opcodeCyclesElapsed = 0;

		uint8_t opcode = this->databus->read(this->registers.PC);  // Get the next opcode.

		// Check if this opcode exists.
		if (!INSTRUCTION_SET.contains(opcode)) {
			return FAIL;
		};
		Instruction& instruction = INSTRUCTION_SET[opcode];
		if (opcode == 0x10) {
			int a = 0;
		}

		this->executeOpcode(opcode);
		this->currentOpcodeCycleLen = instruction.cycleCount;  // Get how many cycles this opcode will be using.

		if (this->interruptRequested) {  // After a request has been made, we do not want to perform the interrupt until after the current opcode is done.
			this->performInterrupt = true;
		}

		this->registers.PC += instruction.numBytes * !instruction.modifiesPC;  // Only move the program counter forward if the instruction does not modify the PC.
		
	}

	++this->totalCyclesElapsed;
	++this->opcodeCyclesElapsed;
	return outcome;
}

void _6502_CPU::requestInterrupt() {
	if (!this->registers.getStatus('I')) {
		this->interruptRequested = true;
	}
}

void _6502_CPU::requestNMI(bool request) {
	this->nmiRequested = request && !this->lastNMISignal;  // The NMI request is only taken if the request was false last time and is true this time (to prevent an NMI being requested over and over).
	this->lastNMISignal = request;
}

void _6502_CPU::reset() {
	// The CPU resets by decrementing the stack pointer by 3 (Going down the stack), setting the PC to the reset vector, and setting the interrupt disable to true.
	// This process takes 7 CPU cycles.
	registers.PC = static_cast<uint16_t>(this->databus->read(RESET_VECTOR_ADDRESS)) + (static_cast<uint16_t>(this->databus->read(RESET_VECTOR_ADDRESS + 1)) << 8);
	registers.SP -= 3;
	registers.setStatus('I', true);
	this->totalCyclesElapsed += 7;

}

void _6502_CPU::powerOn() {
	this->reset();
	this->registers.setStatus('C', 0);
	this->registers.setStatus('Z', 0);
	this->registers.setStatus('D', 0);
	this->registers.setStatus('V', 0);
	this->registers.setStatus('N', 0);
}

void _6502_CPU::performInterruptActions() {
	// First, push the PC + 2 and Status Flags in the stack.
	// NOTE: I don't know if I need to push the current PC, +1, or +2 onto the stack.
	// NOTE: This code is duplicated in instructions.cpp; maybe I can fix that?
	this->databus->write(STACK_END_ADDR + this->registers.SP, this->registers.PC >> 8);  // Store UB of PC (PCH)
	this->databus->write(STACK_END_ADDR + this->registers.SP - 1, this->registers.PC);  // Store LB of PC (PCL), the latter byte is truncated by the cast to uint8_t.
	this->databus->write(STACK_END_ADDR + this->registers.SP - 2, this->registers.S);

	// Then, get the IRQ Interrupt Vector
	// TODO: Get rid of magic numbers.
	// Magic numbers: 0xfffe and 0xffff are the addresses where the IRQ vector is located.
	// lb = lower byte; ub = upper byte.
	uint8_t lb = this->databus->read(0xfffe);
	uint8_t ub = this->databus->read(0xffff);
	uint16_t irqVector = lb + (static_cast<uint16_t>(ub) << 8);

	this->registers.PC = irqVector;
	this->registers.SP -= 3;

	this->registers.setStatus('I', true);
	this->performInterrupt = false;
	this->interruptRequested = false;
}

void _6502_CPU::performNMIActions() {
	// First, push the PC + 2 and Status Flags in the stack.
	// NOTE: I don't know if I need to push the current PC, +1, or +2 onto the stack.
	// NOTE: This code is duplicated in instructions.cpp; maybe I can fix that?
	this->databus->write(STACK_END_ADDR + this->registers.SP, this->registers.PC >> 8);  // Store UB of PC (PCH)
	this->databus->write(STACK_END_ADDR + this->registers.SP - 1, this->registers.PC);  // Store LB of PC (PCL), the latter byte is truncated by the cast to uint8_t.
	this->databus->write(STACK_END_ADDR + this->registers.SP - 2, this->registers.S);

	// Then, get the NMI Interrupt Vector
	// TODO: Get rid of magic numbers.
	// Magic numbers: 0xfffe and 0xffff are the addresses where the IRQ vector is located.
	// lb = lower byte; ub = upper byte.
	uint8_t lb = this->databus->read(0xfffa);
	uint8_t ub = this->databus->read(0xfffb);
	uint16_t nmiVector = lb + (static_cast<uint16_t>(ub) << 8);

	this->registers.PC = nmiVector;
	this->registers.SP -= 3;

	this->registers.setStatus('I', true);
	this->nmiRequested = false;
}

void _6502_CPU::executeOpcode(uint8_t opcode) {
	Instruction& instruction = INSTRUCTION_SET[opcode];  // Here for debugging purposes.
	instruction.performOperation(this->registers, *this->databus);
}

void _6502_CPU::setupInstructionSet() {
	// bne, DEX, EOR,  LDA, LDX, LDY, PHA, PHP, SBC, SEC, SEI, STA, STX, STY, ALL Ts
	
	// ADC (Add with Carry)
	INSTRUCTION_SET[0x69] = Instruction(ops::ADC, addrModes::immediate, 2, 2);
	INSTRUCTION_SET[0x65] = Instruction(ops::ADC, addrModes::zeropage, 2, 3);
	INSTRUCTION_SET[0x75] = Instruction(ops::ADC, addrModes::zeropageX, 2, 4);
	INSTRUCTION_SET[0x6d] = Instruction(ops::ADC, addrModes::absolute, 3, 4);
	INSTRUCTION_SET[0x7d] = Instruction(ops::ADC, addrModes::absoluteX, 3, 4);
	INSTRUCTION_SET[0x79] = Instruction(ops::ADC, addrModes::absoluteY, 3, 4);
	INSTRUCTION_SET[0x61] = Instruction(ops::ADC, addrModes::indirectX, 2, 6);
	INSTRUCTION_SET[0x71] = Instruction(ops::ADC, addrModes::indirectY, 2, 5);

	// AND (Logical AND)
	INSTRUCTION_SET[0x29] = Instruction(ops::AND, addrModes::immediate, 2, 2);
	INSTRUCTION_SET[0x25] = Instruction(ops::AND, addrModes::zeropage, 2, 3);
	INSTRUCTION_SET[0x35] = Instruction(ops::AND, addrModes::zeropageX, 2, 4);
	INSTRUCTION_SET[0x2d] = Instruction(ops::AND, addrModes::absolute, 3, 4);
	INSTRUCTION_SET[0x3d] = Instruction(ops::AND, addrModes::absoluteX, 3, 4);
	INSTRUCTION_SET[0x39] = Instruction(ops::AND, addrModes::absoluteY, 3, 4);
	INSTRUCTION_SET[0x21] = Instruction(ops::AND, addrModes::indirectX, 2, 6);
	INSTRUCTION_SET[0x31] = Instruction(ops::AND, addrModes::indirectY, 2, 5);

	// ASL (Arithmetic Shift Left)
	INSTRUCTION_SET[0x0a] = Instruction((RegOp)ops::ASL, addrModes::accumulator, 1, 2);
	INSTRUCTION_SET[0x06] = Instruction((MemOp)ops::ASL, addrModes::zeropage, 2, 5);
	INSTRUCTION_SET[0x16] = Instruction((MemOp)ops::ASL, addrModes::zeropageX, 2, 6);
	INSTRUCTION_SET[0x0e] = Instruction((MemOp)ops::ASL, addrModes::absolute, 3, 6);
	INSTRUCTION_SET[0x1e] = Instruction((MemOp)ops::ASL, addrModes::absoluteX, 3, 7);

	// Branch Operations (BCC, BCS, BEQ, BMI, BPL, BVC, BVS)
	INSTRUCTION_SET[0x90] = Instruction((BranchOp)ops::BCC, addrModes::relative, 2, 2, true);
	INSTRUCTION_SET[0xb0] = Instruction((BranchOp)ops::BCS, addrModes::relative, 2, 2, true);
	INSTRUCTION_SET[0xf0] = Instruction((BranchOp)ops::BEQ, addrModes::relative, 2, 2, true);
	INSTRUCTION_SET[0x30] = Instruction((BranchOp)ops::BMI, addrModes::relative, 2, 2, true);
	INSTRUCTION_SET[0x10] = Instruction((BranchOp)ops::BPL, addrModes::relative, 2, 2, true);
	INSTRUCTION_SET[0x50] = Instruction((BranchOp)ops::BVC, addrModes::relative, 2, 2, true);
	INSTRUCTION_SET[0x70] = Instruction((BranchOp)ops::BVS, addrModes::relative, 2, 2, true);

	// Bit Test (BIT)
	INSTRUCTION_SET[0x24] = Instruction(ops::BIT, addrModes::zeropage, 2, 3);
	INSTRUCTION_SET[0x2c] = Instruction(ops::BIT, addrModes::absolute, 3, 4);

	// Clear and Set Flag Instructions (CLC, CLI, CLV)
	INSTRUCTION_SET[0x18] = Instruction(ops::CLC, addrModes::implicit, 1, 2);
	INSTRUCTION_SET[0x58] = Instruction(ops::CLI, addrModes::implicit, 1, 2);
	INSTRUCTION_SET[0xb8] = Instruction(ops::CLV, addrModes::implicit, 1, 2);
	INSTRUCTION_SET[0xd8] = Instruction(ops::CLD, addrModes::implicit, 1, 2);

	// Compare (CMP, CPX, CPY)
	INSTRUCTION_SET[0xc9] = Instruction(ops::CMP, addrModes::immediate, 2, 2);
	INSTRUCTION_SET[0xc5] = Instruction(ops::CMP, addrModes::zeropage, 2, 3);
	INSTRUCTION_SET[0xd5] = Instruction(ops::CMP, addrModes::zeropageX, 2, 4);
	INSTRUCTION_SET[0xcd] = Instruction(ops::CMP, addrModes::absolute, 3, 4);
	INSTRUCTION_SET[0xdd] = Instruction(ops::CMP, addrModes::absoluteX, 3, 4);
	INSTRUCTION_SET[0xd9] = Instruction(ops::CMP, addrModes::absoluteY, 3, 4);
	INSTRUCTION_SET[0xc1] = Instruction(ops::CMP, addrModes::indirectX, 2, 6);
	INSTRUCTION_SET[0xd1] = Instruction(ops::CMP, addrModes::indirectY, 2, 5);

	INSTRUCTION_SET[0xe0] = Instruction(ops::CPX, addrModes::immediate, 2, 2);
	INSTRUCTION_SET[0xe4] = Instruction(ops::CPX, addrModes::zeropage, 2, 3);
	INSTRUCTION_SET[0xec] = Instruction(ops::CPX, addrModes::absolute, 3, 4);

	INSTRUCTION_SET[0xc0] = Instruction(ops::CPY, addrModes::immediate, 2, 2);
	INSTRUCTION_SET[0xc4] = Instruction(ops::CPY, addrModes::zeropage, 2, 3);
	INSTRUCTION_SET[0xcc] = Instruction(ops::CPY, addrModes::absolute, 3, 4);

	// Jump (JMP)
	INSTRUCTION_SET[0x4c] = Instruction(ops::JMP, addrModes::absolute, 3, 3, true);
	INSTRUCTION_SET[0x6c] = Instruction(ops::JMP, addrModes::indirect, 3, 5, true);

	// Jump to Subroutine and Return from Subroutine (JSR, RTS)
	INSTRUCTION_SET[0x20] = Instruction(ops::JSR, addrModes::absolute, 3, 6, true);
	INSTRUCTION_SET[0x60] = Instruction(ops::RTS, addrModes::implicit, 1, 6, true);

	// Increment (INC)
	INSTRUCTION_SET[0xe6] = Instruction(ops::INC, addrModes::zeropage, 2, 5);
	INSTRUCTION_SET[0xf6] = Instruction(ops::INC, addrModes::zeropageX, 2, 6);
	INSTRUCTION_SET[0xee] = Instruction(ops::INC, addrModes::absolute, 3, 6);
	INSTRUCTION_SET[0xfe] = Instruction(ops::INC, addrModes::absoluteX, 3, 7);

	// Decrement (DEC)
	INSTRUCTION_SET[0xc6] = Instruction(ops::DEC, addrModes::zeropage, 2, 5);
	INSTRUCTION_SET[0xd6] = Instruction(ops::DEC, addrModes::zeropageX, 2, 6);
	INSTRUCTION_SET[0xce] = Instruction(ops::DEC, addrModes::absolute, 3, 6);
	INSTRUCTION_SET[0xde] = Instruction(ops::DEC, addrModes::absoluteX, 3, 7);

	// Increment and Decrement Index Registers (INX, INY, DEY)
	INSTRUCTION_SET[0xe8] = Instruction(ops::INX, addrModes::implicit, 1, 2);
	INSTRUCTION_SET[0xc8] = Instruction(ops::INY, addrModes::implicit, 1, 2);
	INSTRUCTION_SET[0x88] = Instruction(ops::DEY, addrModes::implicit, 1, 2);

	// Logical Shift Right (LSR)
	INSTRUCTION_SET[0x4a] = Instruction((RegOp)ops::LSR, addrModes::accumulator, 1, 2);
	INSTRUCTION_SET[0x46] = Instruction((MemOp)ops::LSR, addrModes::zeropage, 2, 5);
	INSTRUCTION_SET[0x56] = Instruction((MemOp)ops::LSR, addrModes::zeropageX, 2, 6);
	INSTRUCTION_SET[0x4e] = Instruction((MemOp)ops::LSR, addrModes::absolute, 3, 6);
	INSTRUCTION_SET[0x5e] = Instruction((MemOp)ops::LSR, addrModes::absoluteX, 3, 7);

	// NOP (No Operation)
	INSTRUCTION_SET[0xEA] = Instruction((RegOp)ops::NOP, addrModes::implicit, 1, 2);  // Reg or Mem op; doesn't matter which.

	// ORA (Logical Inclusive OR)
	INSTRUCTION_SET[0x09] = Instruction(ops::ORA, addrModes::immediate, 2, 2);
	INSTRUCTION_SET[0x05] = Instruction(ops::ORA, addrModes::zeropage, 2, 3);
	INSTRUCTION_SET[0x15] = Instruction(ops::ORA, addrModes::zeropageX, 2, 4);
	INSTRUCTION_SET[0x0D] = Instruction(ops::ORA, addrModes::absolute, 3, 4);
	INSTRUCTION_SET[0x1D] = Instruction(ops::ORA, addrModes::absoluteX, 3, 4);
	INSTRUCTION_SET[0x19] = Instruction(ops::ORA, addrModes::absoluteY, 3, 4);
	INSTRUCTION_SET[0x01] = Instruction(ops::ORA, addrModes::indirectX, 2, 6);
	INSTRUCTION_SET[0x11] = Instruction(ops::ORA, addrModes::indirectY, 2, 5);

	// PLA (Pull Accumulator from Stack)
	INSTRUCTION_SET[0x68] = Instruction(ops::PLA, addrModes::implicit, 1, 4);

	// PLP (Pull Processor Status from Stack)
	INSTRUCTION_SET[0x28] = Instruction(ops::PLP, addrModes::implicit, 1, 4);

	// RTI (Return from Interrupt)
	INSTRUCTION_SET[0x40] = Instruction(ops::RTI, addrModes::implicit, 1, 6);

	// ROR (Rotate Right)
	INSTRUCTION_SET[0x6a] = Instruction((RegOp)ops::ROR, addrModes::accumulator, 1, 2);
	INSTRUCTION_SET[0x66] = Instruction((MemOp)ops::ROR, addrModes::zeropage, 2, 5);
	INSTRUCTION_SET[0x76] = Instruction((MemOp)ops::ROR, addrModes::zeropageX, 2, 6);
	INSTRUCTION_SET[0x6e] = Instruction((MemOp)ops::ROR, addrModes::absolute, 3, 6);
	INSTRUCTION_SET[0x7e] = Instruction((MemOp)ops::ROR, addrModes::absoluteX, 3, 7);

	// ROL (Rotate Left)
	INSTRUCTION_SET[0x2a] = Instruction((RegOp)ops::ROL, addrModes::accumulator, 1, 2);
	INSTRUCTION_SET[0x26] = Instruction((MemOp)ops::ROL, addrModes::zeropage, 2, 5);
	INSTRUCTION_SET[0x36] = Instruction((MemOp)ops::ROL, addrModes::zeropageX, 2, 6);
	INSTRUCTION_SET[0x2e] = Instruction((MemOp)ops::ROL, addrModes::absolute, 3, 6);
	INSTRUCTION_SET[0x3e] = Instruction((MemOp)ops::ROL, addrModes::absoluteX, 3, 7);

	// BNE (Branch if Not Equal)
	INSTRUCTION_SET[0xd0] = Instruction((BranchOp)ops::BNE, addrModes::relative, 2, 2, true);

	// DEX (Decrement X)
	INSTRUCTION_SET[0xca] = Instruction(ops::DEX, addrModes::implicit, 1, 2);

	// EOR (Exclusive OR)
	INSTRUCTION_SET[0x49] = Instruction(ops::EOR, addrModes::immediate, 2, 2);
	INSTRUCTION_SET[0x45] = Instruction(ops::EOR, addrModes::zeropage, 2, 3);
	INSTRUCTION_SET[0x55] = Instruction(ops::EOR, addrModes::zeropageX, 2, 4);
	INSTRUCTION_SET[0x4d] = Instruction(ops::EOR, addrModes::absolute, 3, 4);
	INSTRUCTION_SET[0x5d] = Instruction(ops::EOR, addrModes::absoluteX, 3, 4);
	INSTRUCTION_SET[0x59] = Instruction(ops::EOR, addrModes::absoluteY, 3, 4);
	INSTRUCTION_SET[0x41] = Instruction(ops::EOR, addrModes::indirectX, 2, 6);
	INSTRUCTION_SET[0x51] = Instruction(ops::EOR, addrModes::indirectY, 2, 5);

	// LDA (Load Accumulator)
	INSTRUCTION_SET[0xa9] = Instruction(ops::LDA, addrModes::immediate, 2, 2);
	INSTRUCTION_SET[0xa5] = Instruction(ops::LDA, addrModes::zeropage, 2, 3);
	INSTRUCTION_SET[0xb5] = Instruction(ops::LDA, addrModes::zeropageX, 2, 4);
	INSTRUCTION_SET[0xad] = Instruction(ops::LDA, addrModes::absolute, 3, 4);
	INSTRUCTION_SET[0xbd] = Instruction(ops::LDA, addrModes::absoluteX, 3, 4);
	INSTRUCTION_SET[0xb9] = Instruction(ops::LDA, addrModes::absoluteY, 3, 4);
	INSTRUCTION_SET[0xa1] = Instruction(ops::LDA, addrModes::indirectX, 2, 6);
	INSTRUCTION_SET[0xb1] = Instruction(ops::LDA, addrModes::indirectY, 2, 5);

	// LDX (Load X)
	INSTRUCTION_SET[0xa2] = Instruction(ops::LDX, addrModes::immediate, 2, 2);
	INSTRUCTION_SET[0xa6] = Instruction(ops::LDX, addrModes::zeropage, 2, 3);
	INSTRUCTION_SET[0xb6] = Instruction(ops::LDX, addrModes::zeropageY, 2, 4);
	INSTRUCTION_SET[0xae] = Instruction(ops::LDX, addrModes::absolute, 3, 4);
	INSTRUCTION_SET[0xbe] = Instruction(ops::LDX, addrModes::absoluteY, 3, 4);

	// LDY (Load Y)
	INSTRUCTION_SET[0xa0] = Instruction(ops::LDY, addrModes::immediate, 2, 2);
	INSTRUCTION_SET[0xa4] = Instruction(ops::LDY, addrModes::zeropage, 2, 3);
	INSTRUCTION_SET[0xb4] = Instruction(ops::LDY, addrModes::zeropageX, 2, 4);
	INSTRUCTION_SET[0xac] = Instruction(ops::LDY, addrModes::absolute, 3, 4);
	INSTRUCTION_SET[0xbc] = Instruction(ops::LDY, addrModes::absoluteX, 3, 4);

	// PHA (Push Accumulator)
	INSTRUCTION_SET[0x48] = Instruction(ops::PHA, addrModes::implicit, 1, 3);

	// PHP (Push Processor Status)
	INSTRUCTION_SET[0x08] = Instruction(ops::PHP, addrModes::implicit, 1, 3);

	// SBC (Subtract with Carry)
	INSTRUCTION_SET[0xe9] = Instruction(ops::SBC, addrModes::immediate, 2, 2);
	INSTRUCTION_SET[0xe5] = Instruction(ops::SBC, addrModes::zeropage, 2, 3);
	INSTRUCTION_SET[0xf5] = Instruction(ops::SBC, addrModes::zeropageX, 2, 4);
	INSTRUCTION_SET[0xed] = Instruction(ops::SBC, addrModes::absolute, 3, 4);
	INSTRUCTION_SET[0xfd] = Instruction(ops::SBC, addrModes::absoluteX, 3, 4);
	INSTRUCTION_SET[0xf9] = Instruction(ops::SBC, addrModes::absoluteY, 3, 4);
	INSTRUCTION_SET[0xe1] = Instruction(ops::SBC, addrModes::indirectX, 2, 6);
	INSTRUCTION_SET[0xf1] = Instruction(ops::SBC, addrModes::indirectY, 2, 5);

	// SEC (Set Carry Flag)
	INSTRUCTION_SET[0x38] = Instruction(ops::SEC, addrModes::implicit, 1, 2);

	// SEI (Set Interrupt Disable)
	INSTRUCTION_SET[0x78] = Instruction(ops::SEI, addrModes::implicit, 1, 2);

	// SED (Set Decimal Flag)
	INSTRUCTION_SET[0xf8] = Instruction(ops::SED, addrModes::implicit, 1, 2);

	// STA (Store Accumulator)
	INSTRUCTION_SET[0x85] = Instruction(ops::STA, addrModes::zeropage, 2, 3);
	INSTRUCTION_SET[0x95] = Instruction(ops::STA, addrModes::zeropageX, 2, 4);
	INSTRUCTION_SET[0x8d] = Instruction(ops::STA, addrModes::absolute, 3, 4);
	INSTRUCTION_SET[0x9d] = Instruction(ops::STA, addrModes::absoluteX, 3, 5);
	INSTRUCTION_SET[0x99] = Instruction(ops::STA, addrModes::absoluteY, 3, 5);
	INSTRUCTION_SET[0x81] = Instruction(ops::STA, addrModes::indirectX, 2, 6);
	INSTRUCTION_SET[0x91] = Instruction(ops::STA, addrModes::indirectY, 2, 6);

	// STX (Store X)
	INSTRUCTION_SET[0x86] = Instruction(ops::STX, addrModes::zeropage, 2, 3);
	INSTRUCTION_SET[0x96] = Instruction(ops::STX, addrModes::zeropageY, 2, 4);
	INSTRUCTION_SET[0x8e] = Instruction(ops::STX, addrModes::absolute, 3, 4);

	// STY (Store Y)
	INSTRUCTION_SET[0x84] = Instruction(ops::STY, addrModes::zeropage, 2, 3);
	INSTRUCTION_SET[0x94] = Instruction(ops::STY, addrModes::zeropageX, 2, 4);
	INSTRUCTION_SET[0x8c] = Instruction(ops::STY, addrModes::absolute, 3, 4);

	// TXA (Transfer X to A)
	INSTRUCTION_SET[0x8a] = Instruction(ops::TXA, addrModes::implicit, 1, 2);

	// TXS (Transfer X to Stack Pointer)
	INSTRUCTION_SET[0x9a] = Instruction(ops::TXS, addrModes::implicit, 1, 2);

	// TYA (Transfer Y to A)
	INSTRUCTION_SET[0x98] = Instruction(ops::TYA, addrModes::implicit, 1, 2);

	// TAY (Transfer A to Y)
	INSTRUCTION_SET[0xa8] = Instruction(ops::TAY, addrModes::implicit, 1, 2);

	// TAX (Transfer A to X)
	INSTRUCTION_SET[0xaa] = Instruction(ops::TAX, addrModes::implicit, 1, 2);

	// TSX (Transfer Stack Pointer to X)
	INSTRUCTION_SET[0xba] = Instruction(ops::TSX, addrModes::implicit, 1, 2);
}