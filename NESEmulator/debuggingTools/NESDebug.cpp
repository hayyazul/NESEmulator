#include "NESDebug.h"
#include "debugDatabus.h"
#include "CPUAnalyzer.h"
#include <minmax.h>

NESDebug::NESDebug() : NES() {
	// Here, we replace the vanilla componenets w/ their debugger counterparts.
	delete this->ppu;
	delete this->databus;

	this->ppuInstance.attach(this->VRAM);
	this->ppu = &this->ppuInstance;

	this->databusInstance.attach(this->memory);
	this->databusInstance.attach(this->ram);
	this->databusInstance.attach(this->ppu);
	this->databus = &this->databusInstance;
	
	this->cpuInstance.attach(&this->databusInstance);  // Attaches the debugger pointer instead.
	this->CPU = (_6502_CPU*)(&this->cpuInstance);  // NOTE: Why did I add this cast here? Try to remove it if it doesn't break anything.
}

NESDebug::~NESDebug() {
	// The CPU and Databus instances will get destroyed because they are 
	// a part of this class.
}

NESCycleOutcomes NESDebug::executeInstruction() {
	NESCycleOutcomes success;
	do {  // Performs machine cycles until an instruction is executed.
		success = this->executeMachineCycle();
	} while (success != FAIL_CYCLE && success != INSTRUCTION_AND_PPU_CYCLE);
	return success;
}

NESCycleOutcomes NESDebug::executeMachineCycle() {
	int oldMemOpsSize = this->databusInstance.getMemOps()->size();
	NESCycleOutcomes success = NES::executeMachineCycle();
	if (success && this->getRecord()) {
		MachineAction action;
		action.instructionExecuted = this->databusInstance.getMemOps()->size() > oldMemOpsSize;
		// TODO: get PPU actions.
		action.NMIRequested = this->ppu->requestingNMI();  // This also means that an NMI request is made to the CPU.
		
		if (action.NMIRequested) {
			int b = 0;
		}

		action.cycle = this->totalMachineCycles - 1;  // Since NES::executeMachineCycle increments the # of machine cycles.

		//this->databusInstance.clearRecordedActions();

		this->machineActions.push_back(action);
	}

	return success;
}

MachineAction NESDebug::undoInstruction() {
	MachineAction lastAction;
	// Undo machine cycles until we can not undo any more or until we have undone a machine cycle w/ an instruction executed.
	do {
		lastAction = this->undoMachineCycle();
	} while (lastAction.cycle != -1 && lastAction.instructionExecuted != true);

	// When we do undo an instruction, we still need to undo the cycles inbetween the undone instruction and the instruction before that.

	if (lastAction.cycle == -1) {
		return MachineAction();
	} else {
		return lastAction;
	}
}

MachineAction NESDebug::undoMachineCycle() {
	// This emulator does the following (in chronological order):
	// 1. Check if the CPU can execute a cycle.
	//    a. If it can, execute a CPU cycle.
	// 2. Execute a PPU Cycle.
	// 3. Check if the PPU is requesting an NMI.
	//    a. If it is, then inform the CPU of an NMI request.
	
	if (this->totalMachineCycles) {
		MachineAction lastAction = this->machineActions.back();
		if (lastAction.NMIRequested) {
			// TODO
		}

		// TODO: Implement PPU undo.

		// Check if the last machine cycle had a cpu instruction executed or not.
		// if (lastAction.instructionExecuted) {
		//	 this->cpuInstance.undoInstruction();
		// }

		if ((this->totalMachineCycles - 1)% 3 == 0) {
			this->cpuInstance.undoCPUCycle();
		}

		this->machineActions.pop_back();
		--this->totalMachineCycles;

		return lastAction;
	}

	return MachineAction();
}

std::vector<MachineAction> NESDebug::getMachineActions() {
	return this->machineActions;
}

std::vector<MachineAction> NESDebug::getMachineActions(int numOfActions) {
	std::vector<MachineAction> machineActions;
	int numOfActionsToGet = min(numOfActions, this->machineActions.size());
	
	for (int i = this->machineActions.size() - numOfActionsToGet; i < this->machineActions.size(); ++i) {
		machineActions.push_back(this->machineActions.at(i));
	}

	return machineActions;
}

bool NESDebug::setRecord(bool record) {
	bool oldRecordActions = this->databusInstance.getRecordActions();
	this->databusInstance.setRecordActions(record);
	this->cpuInstance.setRecordActions(record);
	return oldRecordActions;
}

bool NESDebug::getRecord() const {
	return this->databusInstance.getRecordActions();
}

void NESDebug::clearRecord() {
	while (!this->machineActions.empty()) {
		this->machineActions.pop_back();
	}
	this->cpuInstance.clearExecutedInstructions();
}

void NESDebug::setStdValue(uint8_t val) {
	for (unsigned int i = 0; i < BYTES_OF_MEMORY; ++i) {
		this->memory->setByte(i, val);
	}
}

uint8_t NESDebug::memPeek(uint16_t memoryAddress) {
	return this->databus->read(memoryAddress);
}
void NESDebug::performMachineAction(MachineAction machineAction, bool reverseOrder) {

}
CPUDebugger* NESDebug::getCPUPtr() {
	CPUDebugger* cpuPtr = &this->cpuInstance;
	return cpuPtr;
}
PPUDebug* NESDebug::getPPUPtr() {
	PPUDebug* ppuPtr = &this->ppuInstance;
	return ppuPtr;
}
/*
Registers NESDebug::registersPeek() {
	return this->CPU.registersPeek();
}

bool NESDebug::registersPeek(char c) {
	return this->CPU.registersPeek().getStatus(c);
}

void NESDebug::registersPoke(Registers registers) {
	this->CPU.registersPoke(registers);
}

void NESDebug::memPoke(uint16_t memoryAddress, uint8_t val) {
	this->CPU.memPoke(memoryAddress, val);
}

bool NESDebug::memFind(uint8_t value, uint16_t& address, int lowerBound, int upperBound) {
	return this->CPU.memFind(value, address, lowerBound, upperBound);
}
*/