// instructions.h : Holds all the operations and addressing modes; using
// one function from each group you can make an Instruction.

#pragma once
#include <cstdint>

enum AddressingModes;
struct Registers;
class DataBus;


typedef void(*RegOp)(Registers& registers, uint8_t data);
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

typedef uint8_t(*DataFetcher)(DataBus& databus, Registers& registers);
typedef uint16_t(*AddressFetcher)(DataBus& databus, Registers& registers);
union AddressingOperation {
	DataFetcher dataFetcher;
	AddressFetcher addressFetcher;

	uint16_t operator()(DataBus& databus, Registers& registers, bool fetchAddress=false) {
		if (fetchAddress) {
			return addressFetcher(databus, registers);
		}
		else {
			return dataFetcher(databus, registers);
		}
	}
};

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

	Instruction() : size(0), cycleCount(0) {};
	Instruction(RegOp op,
		DataFetcher addrOp,
		unsigned int size,
		unsigned int cycleCount)
		:
		size(size),
		cycleCount(cycleCount),
		isMemOp(false)
	{
		this->operation.regOp = op;
		this->addresser.dataFetcher = addrOp;
	};
	Instruction(MemOp op,
		AddressFetcher addrOp,
		unsigned int size,
		unsigned int cycleCount)
		:
		size(size),
		cycleCount(cycleCount),
		isMemOp(true) 
	{
		this->operation.memOp = op;
		this->addresser.addressFetcher = addrOp;
	};

	~Instruction() {};

	uint16_t addressingOperation(DataBus& databus, Registers& registers) {
		return this->addresser(databus, registers, this->isMemOp);
	}
};

namespace flagOps {
	bool isSignBitIncorrect(uint8_t aBefore, uint8_t sum, uint8_t data);
	bool isBit7Set(uint8_t byte);
	bool isOverflow(uint8_t a, uint8_t data, bool c);
	bool isUnderflow(uint8_t a, uint8_t data, bool c);
};

namespace ops {  // ops = operations
	void ADC(Registers& registers, uint8_t data);
	void AND(Registers& registers, uint8_t data);
	void ASL(Registers& registers, uint8_t data);
	void BCC(Registers& registers, uint8_t data);
	void BCS(Registers& registers, uint8_t data);
	void BEQ(Registers& registers, uint8_t data);
	void BIT(Registers& registers, uint8_t data);

	void LDA(Registers& registers, uint8_t data);
	void STA(Registers& registers, DataBus& databus, uint16_t data);
};

// TODO: implement ability to get 16 bit addresses via an addressing mode.
// NOTE: I might change how addressing works entirely; instead of fetching data, it could fetch a memory address to get data from. Then the 
// operation can get the data as it needs; a single byte for most operations (the address returned), or two bytes for memory-related ones
// (e.g. STA instructions).
namespace dataAddrOp {
	/*
		The following are the various addressing operations an operation can use.
	An example in assembly is provided right of all addressing modes (OPC LLLLL) 
	if applicable.
		Note that a '$' indicates the value is in hex. They are not required and do not 
	affect the addressing mode.
	*/
	
	uint8_t immediate(DataBus& databus, Registers& registers);  // LDA #$7b
	uint8_t implicit(DataBus& databus, Registers& registers);  // CLC [N/A]
	uint8_t accumulator(DataBus& databus, Registers& registers);  // LSR [N/A]
	
	uint8_t zeropage(DataBus& databus, Registers& registers);  // STX $32
	uint8_t zeropageX(DataBus& databus, Registers& registers);  // STY $32,X
	uint8_t zeropageY(DataBus& databus, Registers& registers);  // LDX $10,Y
	uint8_t relative(DataBus& databus, Registers& registers);  // BNE *+4 
	uint8_t absolute(DataBus& databus, Registers& registers);  // JMP $1234
	uint8_t absoluteX(DataBus& databus, Registers& registers);  // STA $3000,X
	uint8_t absoluteY(DataBus& databus, Registers& registers);  // AND $4000,Y

	// Works similar to pointers. It first goes to the given memory address, looks at the
	// byte and the byte of the next address, uses those two bytes to make a new address 
	// which it gets the value of.
	uint8_t indirect(DataBus& databus, Registers& registers);  // JMP ($4321)
	uint8_t indirectX(DataBus& databus, Registers& registers);  // STA ($40,X)
	uint8_t indirectY(DataBus& databus, Registers& registers);  // LDA ($20),Y
}

namespace addr16bitOp {
	uint16_t zeropage(DataBus& databus, Registers& registers);
}