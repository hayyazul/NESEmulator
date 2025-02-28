// instructions.h : Holds all the operations and addressing modes; using
// one function from each group you can make an Instruction.

// NOTE: I mixed up instructions and opcodes; opcode refers to the specific byte in machine code which in turn corresponds to something like STA w/ zero page addressing. 
// An instruction on the other hand focuses only on the thing being done, which in the above case would just be STA.
#pragma once
#include <cstdint>
#include "../databus/databus.h"

enum AddressingModes;
struct Registers;

typedef void(*RegOp)(Registers& registers, uint8_t data);  // Operations which work with data and the registers.
typedef void(*MemOp)(Registers& registers, DataBus& databus, uint16_t address);  // Operations which work with addresses (thus, it also needs the databus) and registers.
typedef void(*BranchOp)(Registers& registers, uint16_t address, bool& branched);  // Operations which branch.
union Operation {
	RegOp regOp;
	MemOp memOp;
	BranchOp branchOp;

	void operator()(Registers& registers, uint8_t data) {
		regOp(registers, data);
	}
	void operator()(Registers& registers, DataBus& databus, uint16_t address) {
		memOp(registers, databus, address);
	}
	void operator()(Registers& registers, uint8_t data, bool& branched) {
		branchOp(registers, data, branched);
	}
};

typedef uint16_t(*Addresser)(DataBus& databus, Registers& registers);
typedef uint16_t(*CycleChangingAddresser)(DataBus& databus, Registers& registers, bool& addCycles);

union AddressingOperation {
	Addresser addresser;
	CycleChangingAddresser cCAddresser;

	uint16_t operator()(DataBus& databus, Registers& registers) {
		return addresser(databus, registers);
	}
	uint16_t operator()(DataBus& databus, Registers& registers, bool& addCycles) {
		return cCAddresser(databus, registers, addCycles);
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

enum OpType {
	MEM,
	REG,
	BRANCH
};

/* struct Instruction
	An Instruction is made up of an operation and an addressing operation.
	Should have made this a class.
*/
struct Instruction {
	Operation operation;
	AddressingOperation addresser;
	OpType opType;

	bool modifiesPC, pgCrossingDependent;
	unsigned int numBytes;
	unsigned int cycleCount, baseCycleCount;

	/*
	Operation operation - Function pointer to the thing which performs an operation on the given data or addresses.
	AddressingOperation addresser - Thing that gets the address to use or the address to get data from (depending on operation).
	bool:
	modifiesPC - Flag for whether this instruction affects the program counter directly; useful for program control instructions
	where you want to make sure the CPU does not affect the PC itself.
	pgDependent - Whether this instruction has an extra cycle whenever its AbsX, AbsY, and or (Ind)Y addressers cross
	a page boundary (each page is 256 bytes long; from 0xnn00 to 0xnnff).
	
	unsigned int:
	numBytes - Size of this instruction including operands.
	cycleCount - How many cycles this instruction used.
	*/

	Instruction() {};
	Instruction(RegOp op,
		Addresser addrOp,
		unsigned int size,
		unsigned int cycleCount,
		bool modifiesPC = false)
		:
		numBytes(size),
		baseCycleCount(cycleCount),
		cycleCount(cycleCount),
		opType(REG),
		modifiesPC(modifiesPC),
		pgCrossingDependent(false)
	{
		this->operation.regOp = op;
		this->addresser.addresser = addrOp;
	};
	Instruction(RegOp op,
		CycleChangingAddresser addrOp,
		unsigned int size,
		unsigned int cycleCount,
		bool modifiesPC = false)
		:
		numBytes(size),
		baseCycleCount(cycleCount),
		cycleCount(cycleCount),
		opType(REG),
		modifiesPC(modifiesPC),
		pgCrossingDependent(true)
	{
		this->operation.regOp = op;
		this->addresser.cCAddresser = addrOp;
	};
	Instruction(MemOp op,
		Addresser addrOp,
		unsigned int size,
		unsigned int cycleCount,
		bool modifiesPC = false)
		:
		numBytes(size),
		baseCycleCount(cycleCount),
		cycleCount(cycleCount),
		opType(MEM),
		modifiesPC(modifiesPC),
		pgCrossingDependent(false)
	{
		this->operation.memOp = op;
		this->addresser.addresser = addrOp;
	};
	Instruction(MemOp op,
		CycleChangingAddresser addrOp,
		unsigned int size,
		unsigned int cycleCount,
		bool modifiesPC = false)
		:
		numBytes(size),
		baseCycleCount(cycleCount),
		cycleCount(cycleCount),
		opType(MEM),
		modifiesPC(modifiesPC),
		pgCrossingDependent(true)
	{
		this->operation.memOp = op;
		this->addresser.cCAddresser = addrOp;
	};
	Instruction(BranchOp op,
		CycleChangingAddresser addrOp,
		unsigned int size,
		unsigned int cycleCount,
		bool modifiesPC = false)
		:
		numBytes(size),
		baseCycleCount(cycleCount),
		cycleCount(cycleCount),
		opType(BRANCH),
		modifiesPC(modifiesPC),
		pgCrossingDependent(true)
	{
		this->operation.branchOp = op;
		this->addresser.cCAddresser = addrOp;
	};

	~Instruction() {};

	// TODO: instead of changing this->cyclesLen..., make this return the # of cycles it took.
	void performOperation(Registers& registers, DataBus& databus) {	
		uint16_t address;
		this->cycleCount = this->baseCycleCount;

		bool pgCross = false;
		if (this->pgCrossingDependent) {
			address = this->addresser(databus, registers, pgCross);
		} else {
			address = this->addresser(databus, registers);
		}
		// If the operation uses the address for more than just getting the data, pass the address.
		uint8_t data;
		bool branchSuccessful = false;
		switch(this->opType) {
		case(MEM):
			this->operation.memOp(registers, databus, address);
			break;
		case(REG):
			data = databus.read(address);
			this->operation.regOp(registers, data);

			this->cycleCount += pgCross;
			break;
		case(BRANCH):
			data = databus.read(address);
			this->operation.branchOp(registers, data, branchSuccessful);
			
			this->cycleCount += pgCross * branchSuccessful;  // Even if a branch instruction crosses a page, if it never branches, do NOT add cycles.
			this->cycleCount += branchSuccessful;
			break;
		default:
			break;
		}
 	}
};

namespace helperByteOps {
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
	// For branch instructions, since they modify the PC and thus will tell the CPU to not iterate it, they have to modify it themselves.
	// All branch instructions use one addressing mode (relative) and all have a fixed length of 2 bytes; thus we add 2 if the branch condition fails.
	// The relative addressing adds or subtracts the PC; it does NOT account for the length of the opcode, so keep that in mind.
	// In other words, when branching FORWARDS, add two. Do NOT add two when branching backwards.
		void BCC(Registers& registers, uint8_t data, bool& branched);
		void BCS(Registers& registers, uint8_t data, bool& branched);
		void BEQ(Registers& registers, uint8_t data, bool& branched);
	void BIT(Registers& registers, uint8_t data);
		void BMI(Registers& registers, uint8_t data, bool& branched);
		void BNE(Registers& registers, uint8_t data, bool& branched);
		void BPL(Registers& registers, uint8_t data, bool& branched);
	void BRK(Registers& registers, DataBus& databus, uint16_t data);
		void BVC(Registers& registers, uint8_t data, bool& branched);
		void BVS(Registers& registers, uint8_t data, bool& branched);
	void CLC(Registers& registers, uint8_t data);
	void CLD(Registers& registers, uint8_t data);
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

	void JMP(Registers& registers, DataBus& databus, uint16_t data);
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
	void SED(Registers& registers, uint8_t data);
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
	uint16_t relative(DataBus& databus, Registers& registers, bool& addCycles);  // BNE *+4 
	uint16_t absolute(DataBus& databus, Registers& registers);  // JMP $1234
	uint16_t absoluteX(DataBus& databus, Registers& registers, bool& addCycles);  // STA $3000,X
	uint16_t absoluteY(DataBus& databus, Registers& registers, bool& addCycles);  // AND $4000,Y

	// Works similar to pointers. It first goes to the given memory address, looks at the
	// byte and the byte of the next address, uses those two bytes to make a new address 
	// which it gets the value of.
	uint16_t indirect(DataBus& databus, Registers& registers);  // JMP ($4321)
	uint16_t indirectX(DataBus& databus, Registers& registers);  // STA ($40,X)
	uint16_t indirectY(DataBus& databus, Registers& registers, bool& addCycles);  // LDA ($20),Y
}