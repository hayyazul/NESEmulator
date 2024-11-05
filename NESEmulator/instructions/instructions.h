// instructions.h : Holds all the operations and addressing modes; using
// one function from each group you can make an Instruction.

// NOTE: I mixed up instructions and opcodes; opcode refers to the specific byte in machine code which in turn corresponds to something like STA w/ zero page addressing. 
// An instruction on the other hand focuses only on the thing being done, which in the above case would just be STA.
// TODO: Fix terminology. 

#pragma once
#include <cstdint>
#include "../databus/databus.h"

enum AddressingModes;
struct Registers;

typedef void(*RegOp)(Registers& registers, uint8_t data);  // Operations which work with data and the registers.
typedef void(*MemOp)(Registers& registers, DataBus& databus, uint16_t address);  // Operations which work with addresses (thus, it also needs the databus) and registers.
union Operation {
	RegOp regOp;
	MemOp memOp;

	void operator()(Registers& registers, uint8_t data) {
		regOp(registers, data);
	}
	void operator()(Registers& registers, DataBus& databus, uint16_t address) {
		memOp(registers, databus, address);
	}
};

// GIT BRANCH (always-return-address): Here, the address of the byte in question is fetched--- never the data value itself, that is figured out differently.
typedef uint16_t(*AddressingOperation)(DataBus& databus, Registers& registers);

enum AddressingModes {
	IMPLICIT,
	ACCUMULATOR,
	IMMEDIATE,
	ZERO_PAGE,
	ZERO_PAGE_X,
	ZERO_PAGE_Y,
	RELATIVE,
	ABSOLUTE,
	ABSOLUTE_X,
	ABSOLUTE_Y,
	INDIRECT,
	INDIRECT_X,
	INDIRECT_Y
};

/* struct Instruction
	An Instruction is made up of an operation and an addressing operation.
*/
struct Instruction {
	Operation operation;
	AddressingOperation addresser;
	bool isMemOp, modifiesPC;

	unsigned int numBytes;
	unsigned int cycleCount;

	Instruction() {};
	Instruction(RegOp op,
		AddressingOperation addrOp,
		unsigned int size,
		unsigned int cycleCount,
		bool modifiesPC = false)
		:
		numBytes(size),
		cycleCount(cycleCount),
		isMemOp(false),
		modifiesPC(modifiesPC)
	{
		this->operation.regOp = op;
		this->addresser = addrOp;
	};
	Instruction(MemOp op,
		AddressingOperation addrOp,
		unsigned int size,
		unsigned int cycleCount,
		bool modifiesPC = false)
		:
		numBytes(size),
		cycleCount(cycleCount),
		isMemOp(true),
		modifiesPC(modifiesPC)
	{
		this->operation.memOp = op;
		this->addresser = addrOp;
	};

	~Instruction() {};

	void performOperation(Registers& registers, DataBus& databus) {	
		uint16_t address = this->addresser(databus, registers);

		// If the operation uses the address for more than just getting the data, pass the address.
		if (this->isMemOp) {
			this->operation.memOp(registers, databus, address);
		} else {
			uint8_t data = databus.read(address);
			this->operation.regOp(registers, data);
		}
	}
};

namespace flagOps {
	bool isSignBitIncorrect(uint8_t aBefore, uint8_t sum, uint8_t data);
	bool isBit7Set(uint8_t byte);
	bool isBit0Set(uint8_t byte);
	bool isOverflow(uint8_t a, uint8_t data, bool c);
	bool isUnderflow(uint8_t a, uint8_t data, bool c);
};


// There are 54 operations; there are 56 on a vanilla 6502, but the one the NES uses omits decimal mode, so operations relating to that are not present.
namespace ops {  // ops = operations
	void AND(Registers& registers, uint8_t data);
	void ADC(Registers& registers, uint8_t data);
	void ASL(Registers& registers, uint8_t data);
	void ASL(Registers& registers, DataBus& databus, uint16_t data);
	void BCC(Registers& registers, uint8_t data);
	void BCS(Registers& registers, uint8_t data);
	void BEQ(Registers& registers, uint8_t data);
	void BIT(Registers& registers, uint8_t data);
	void BMI(Registers& registers, uint8_t data);
	void BNE(Registers& registers, uint8_t data);
	void BPL(Registers& registers, uint8_t data);
	void BRK(Registers& registers, DataBus& databus, uint16_t data);
	void BVC(Registers& registers, uint8_t data);
	void BVS(Registers& registers, uint8_t data);
	void CLC(Registers& registers, uint8_t data);
	// CLD is not included because decimal mode does not exist on the NES CPU.
	void CLI(Registers& registers, uint8_t data);
	void CLV(Registers& registers, uint8_t data);
	void CMP(Registers& registers, uint8_t data);
	void CPX(Registers& registers, uint8_t data);
	void CPY(Registers& registers, uint8_t data);
	void DEC(Registers& registers, DataBus& databus, uint16_t data);
	void DEX(Registers& registers, uint8_t data);
	void DEY(Registers& registers, uint8_t data);
	void EOR(Registers& registers, uint8_t data);
	void INC(Registers& registers, DataBus& databus, uint16_t data);
	void INX(Registers& registers, uint8_t data);
	void INY(Registers& registers, uint8_t data);

	void JMP(Registers& registers, uint8_t data);
	void JSR(Registers& registers, DataBus& databus, uint16_t data);

	void LDA(Registers& registers, uint8_t data);
	void LDX(Registers& registers, uint8_t data);
	void LDY(Registers& registers, uint8_t data);
	void LSR(Registers& registers, uint8_t data);
	void LSR(Registers& registers, DataBus& databus, uint16_t data);
	void NOP(Registers& registers, uint8_t data);
	void NOP(Registers& registers, DataBus& databus, uint16_t data);
	
	void ORA(Registers& registers, uint8_t data);
	void PHA(Registers& registers, DataBus& databus, uint16_t data);
	void PHP(Registers& registers, DataBus& databus, uint16_t data);
	void PLA(Registers& registers, DataBus& databus, uint16_t data);
	void PLP(Registers& registers, DataBus& databus, uint16_t data);

	void ROL(Registers& registers, uint8_t data);
	void ROL(Registers& registers, DataBus& databus, uint16_t data);
	void ROR(Registers& registers, uint8_t data);
	void ROR(Registers& registers, DataBus& databus, uint16_t data);
	void RTI(Registers& registers, DataBus& databus, uint16_t data);
	void RTS(Registers& registers, DataBus& databus, uint16_t data);
	void SBC(Registers& registers, uint8_t data);
	void SEC(Registers& registers, uint8_t data);
	void SEI(Registers& registers, uint8_t data);
	
	void STA(Registers& registers, DataBus& databus, uint16_t data);

	void STX(Registers& registers, DataBus& databus, uint16_t data);
	void STY(Registers& registers, DataBus& databus, uint16_t data);
	void TAX(Registers& registers, uint8_t data);
	void TAY(Registers& registers, uint8_t data);
	void TSX(Registers& registers, uint8_t data);
	void TXA(Registers& registers, uint8_t data);
	void TXS(Registers& registers, uint8_t data);
	void TYA(Registers& registers, uint8_t data);
};

// (e.g. STA instructions).
namespace addrModes {
	/*
		The following are the various addressing operations an operation can use.
	An example in assembly is provided right of all addressing modes (OPC LLLLL) 
	if applicable.
		Note that a '$' indicates the value is in hex. They are not required and do not 
	affect the addressing mode.
	*/
	
	uint16_t immediate(DataBus& databus, Registers& registers);  // LDA #$7b
	uint16_t implicit(DataBus& databus, Registers& registers);  // CLC [N/A]
	uint16_t accumulator(DataBus& databus, Registers& registers);  // LSR [N/A]
	
	uint16_t zeropage(DataBus& databus, Registers& registers);  // STX $32
	uint16_t zeropageX(DataBus& databus, Registers& registers);  // STY $32,X
	uint16_t zeropageY(DataBus& databus, Registers& registers);  // LDX $10,Y
	uint16_t relative(DataBus& databus, Registers& registers);  // BNE *+4 
	uint16_t absolute(DataBus& databus, Registers& registers);  // JMP $1234
	uint16_t absoluteX(DataBus& databus, Registers& registers);  // STA $3000,X
	uint16_t absoluteY(DataBus& databus, Registers& registers);  // AND $4000,Y

	// Works similar to pointers. It first goes to the given memory address, looks at the
	// byte and the byte of the next address, uses those two bytes to make a new address 
	// which it gets the value of.
	uint16_t indirect(DataBus& databus, Registers& registers);  // JMP ($4321)
	uint16_t indirectX(DataBus& databus, Registers& registers);  // STA ($40,X)
	uint16_t indirectY(DataBus& databus, Registers& registers);  // LDA ($20),Y
}