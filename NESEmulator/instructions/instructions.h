// instructions.h : Holds all the operations and addressing modes; using
// one function from each group you can make an Instruction.

#pragma once
#include <cstdint>

enum AddressingModes;
struct Registers;
class DataBus;


typedef void(*RegOp)(Registers& registers, uint16_t data);
typedef void(*MemOp)(Registers& registers, DataBus& databus, uint16_t address);
union Operation {
	RegOp regOp;
	MemOp memOp;

	void operator()(Registers& registers, uint8_t data) {
		return regOp(registers, data);
	}
	void operator()(Registers& registers, DataBus& databus, uint16_t address) {
		return memOp(registers, databus, address);
	}
};


// NOTE: Might go back to using a union again.
typedef uint16_t(*AddressingOperation)(DataBus& databus, Registers& registers, bool fetchTwoBytes);
/*union AddressingOperation {
	DataFetcher dataFetcher;

};*/

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

// An opcode which fetches data every cycle or performs an operation.
// TODO: Comments.
struct Instruction {
	Operation operation;
	AddressingOperation addresser;
	bool isMemOp;

	unsigned int size;
	unsigned int cycleCount;

	Instruction() {};
	Instruction(RegOp op,
		AddressingOperation addrOp,
		unsigned int size,
		unsigned int cycleCount)
		:
		size(size),
		cycleCount(cycleCount),
		isMemOp(false)
	{
		this->operation.regOp = op;
		this->addresser = addrOp;
	};
	Instruction(MemOp op,
		AddressingOperation addrOp,
		unsigned int size,
		unsigned int cycleCount)
		:
		size(size),
		cycleCount(cycleCount),
		isMemOp(true)
	{
		this->operation.memOp = op;
		this->addresser = addrOp;
	};

	~Instruction() {};

	void performOperation(Registers& registers, DataBus& databus, uint16_t data) {	
		if (this->isMemOp) {
			this->operation.memOp(registers, databus, data);
		} else {
			this->operation.regOp(registers, static_cast<uint8_t>(data));
		}
	}

	uint16_t addressingOperation(DataBus& databus, Registers& registers) {
		return this->addresser(databus, registers, this->isMemOp);
	}
};

namespace flagOps {
	bool isSignBitIncorrect(uint8_t aBefore, uint8_t sum, uint8_t data);
	bool isBit7Set(uint8_t byte);
	bool isBit0Set(uint8_t byte);
	bool isOverflow(uint8_t a, uint8_t data, bool c);
	bool isUnderflow(uint8_t a, uint8_t data, bool c);
};


// VERY IMPORTANT TODO: Sometimes, data is used as an address by an opcode even though the addressing operations only return data. Change this.
namespace ops {  // ops = operations
	void AND(Registers& registers, uint16_t data);
	void ADC(Registers& registers, uint16_t data);
	void ASL(Registers& registers, uint16_t data);
	void ASL(Registers& registers, DataBus& databus, uint16_t data);
	void BCC(Registers& registers, uint16_t data);
	void BCS(Registers& registers, uint16_t data);
	void BEQ(Registers& registers, uint16_t data);
	void BIT(Registers& registers, uint16_t data);
	void BMI(Registers& registers, uint16_t data);
	void BNE(Registers& registers, uint16_t data);
	void BPL(Registers& registers, uint16_t data);
	void BRK(Registers& registers, DataBus& databus, uint16_t data);
	void BVC(Registers& registers, uint16_t data);
	void BVS(Registers& registers, uint16_t data);
	void CLC(Registers& registers, uint16_t data);
	// CLD is not included because decimal mode does not exist on the NES CPU.
	void CLI(Registers& registers, uint16_t data);
	void CLV(Registers& registers, uint16_t data);
	void CMP(Registers& registers, uint16_t data);
	void CPX(Registers& registers, uint16_t data);
	void CPY(Registers& registers, uint16_t data);
	void DEC(Registers& registers, DataBus& databus, uint16_t data);
	void DEX(Registers& registers, uint16_t data);
	void DEY(Registers& registers, uint16_t data);
	void EOR(Registers& registers, uint16_t data);
	void INC(Registers& registers, DataBus& databus, uint16_t data);
	void INX(Registers& registers, uint16_t data);
	void INY(Registers& registers, uint16_t data);

	void JMP(Registers& registers, uint16_t data);
	void JSR(Registers& registers, DataBus& databus, uint16_t data);

	void LDA(Registers& registers, uint16_t data);
	void LDX(Registers& registers, uint16_t data);
	void LDY(Registers& registers, uint16_t data);
	void LSR(Registers& registers, uint16_t data);
	void LSR(Registers& registers, DataBus& databus, uint16_t data);
	void NOP(Registers& registers, uint16_t data);
	void NOP(Registers& registers, DataBus& databus, uint16_t data);
	
	void ORA(Registers& registers, uint16_t data);
	void PHA(Registers& registers, DataBus& databus, uint16_t data);
	void PHP(Registers& registers, DataBus& databus, uint16_t data);
	void PLA(Registers& registers, DataBus& databus, uint16_t data);
	void PLP(Registers& registers, DataBus& databus, uint16_t data);

	void ROL(Registers& registers, uint16_t data);
	void ROL(Registers& registers, DataBus& databus, uint16_t data);
	void ROR(Registers& registers, uint16_t data);
	void ROR(Registers& registers, DataBus& databus, uint16_t data);
	void RTI(Registers& registers, DataBus& databus, uint16_t data);
	void RTS(Registers& registers, DataBus& databus, uint16_t data);
	void SBC(Registers& registers, uint16_t data);
	void SEC(Registers& registers, uint16_t data);
	void SEI(Registers& registers, uint16_t data);
	
	void STA(Registers& registers, DataBus& databus, uint16_t data);

	void STX(Registers& registers, DataBus& databus, uint16_t data);
	void STY(Registers& registers, DataBus& databus, uint16_t data);
	void TAX(Registers& registers, uint16_t data);
	void TAY(Registers& registers, uint16_t data);
	void TSX(Registers& registers, uint16_t data);
	void TXA(Registers& registers, uint16_t data);
	void TXS(Registers& registers, uint16_t data);
	void TYA(Registers& registers, uint16_t data);
};

// TODO: implement ability to get 16 bit addresses via an addressing mode.
// NOTE: I might change how addressing works entirely; instead of fetching data, it could fetch a memory address to get data from. Then the 
// operation can get the data as it needs; a single byte for most operations (the address returned), or two bytes for memory-related ones
// (e.g. STA instructions).
namespace addrModes {
	/*
		The following are the various addressing operations an operation can use.
	An example in assembly is provided right of all addressing modes (OPC LLLLL) 
	if applicable.
		Note that a '$' indicates the value is in hex. They are not required and do not 
	affect the addressing mode.
	*/
	
	uint16_t immediate(DataBus& databus, Registers& registers, bool fetchTwoBytes = false);  // LDA #$7b
	uint16_t implicit(DataBus& databus, Registers& registers, bool fetchTwoBytes = false);  // CLC [N/A]
	uint16_t accumulator(DataBus& databus, Registers& registers, bool fetchTwoBytes = false);  // LSR [N/A]
	
	uint16_t zeropage(DataBus& databus, Registers& registers, bool fetchTwoBytes = false);  // STX $32
	uint16_t zeropageX(DataBus& databus, Registers& registers, bool fetchTwoBytes = false);  // STY $32,X
	uint16_t zeropageY(DataBus& databus, Registers& registers, bool fetchTwoBytes = false);  // LDX $10,Y
	uint16_t relative(DataBus& databus, Registers& registers, bool fetchTwoBytes = false);  // BNE *+4 
	uint16_t absolute(DataBus& databus, Registers& registers, bool fetchTwoBytes = false);  // JMP $1234
	uint16_t absoluteX(DataBus& databus, Registers& registers, bool fetchTwoBytes = false);  // STA $3000,X
	uint16_t absoluteY(DataBus& databus, Registers& registers, bool fetchTwoBytes = false);  // AND $4000,Y

	// Works similar to pointers. It first goes to the given memory address, looks at the
	// byte and the byte of the next address, uses those two bytes to make a new address 
	// which it gets the value of.
	uint16_t indirect(DataBus& databus, Registers& registers, bool fetchTwoBytes = false);  // JMP ($4321)
	uint16_t indirectX(DataBus& databus, Registers& registers, bool fetchTwoBytes = false);  // STA ($40,X)
	uint16_t indirectY(DataBus& databus, Registers& registers, bool fetchTwoBytes = false);  // LDA ($20),Y
}