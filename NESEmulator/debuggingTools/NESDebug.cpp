#include "NESDebug.h"
#include "debugDatabus.h"
#include "CPUAnalyzer.h"
#include <minmax.h>
#include <sstream>

NESDebug::NESDebug() : NES() {
	NES::attachCartridgeMemory(&this->debugMemory);
	NES::attachCPU(&this->debugCPU);
	NES::attachDataBus(&this->debugDatabus);
	NES::attachPPU(&this->debugPPU);
	NES::attachRAM(&this->debugRAM);
	NES::attachVRAM(&this->debugVRAM);
}
/*
NESDebug::NESDebug(NESDatabus* databus, _6502_CPU* CPU, RAM* ram, Memory* vram, PPU* ppu) 
: NES(databus, CPU, ram, vram, ppu) {}
*/

NESDebug::~NESDebug() {
	// The CPU and Databus instances will get destroyed because they are 
	// a part of this class.
}

NESCycleOutcomes NESDebug::executeMachineCycle() {
	NESCycleOutcomes result = NES::executeMachineCycle();
	return result;
}

bool NESDebug::frameFinished() const {
	bool atEndOfFrame = this->debugPPU.getPosition().inRange(239, 239, 256, 256);
	return atEndOfFrame;
}


void NESDebug::loadInternals(NESInternals internals) {
	this->debugCPU.loadInternals(internals.cpuInternals);
	this->debugPPU.loadInternals(internals.ppuInternals);
	this->DMAUnit = internals.oamDMAUnit;
	*this->ram = internals.ram;
}

PPUInternals NESDebug::getPPUInternals() const {
	// TODO: Complete implementation;
	PPUInternals ppuInternals = debugPPU.getInternals();
	return ppuInternals;
}

CPUInternals NESDebug::getCPUInternals() const {
	return this->debugCPU.getInternals();
}

OAMDMAUnit NESDebug::getOAMDMAUnit() const {
	return this->DMAUnit;
}

void NESDebug::getRAM(RAM& ram) {
	ram = this->debugRAM;
}

void NESDebug::getVRAM(Memory& vram) {
	vram = this->debugVRAM;
}

NESInternals::NESInternals() {
}

NESInternals::~NESInternals() {}

int NESInternals::getMachineCycles() const {
	return this->ppuInternals.cycleCount;
}

std::string NESInternals::getSerialFormat() const {
	/* FORMAT:
	* 
	* cpuInternals
	* ppuInternals
	* OAMDMAUnit
	* RAM
	* 
	* Each unit will be seperated by 2 lines. Each item in each unit will be seperated by a line.
	* It will also have a label preceeding the variable it is saving. For example,
	* 
	* Registers: 1234 [PC], 10 [A], [etc...]
	* OR
	* lastNMISignal: 1
	* 
	* Note that anything inside square brackets and the square brackets themselves are not a part of the string.
	* Also note that nothing is stored as hex; everything will be stored as decimal.
	* 
	* Lastly, also note how any sub-sub elements (PC is a sub element of Registers, which is in turn a sub element of cpuInternals)
	* are seperated by comma.
	* 
	* When loading in this data, there will be a map of various labels to a single label. This is done so if a variable name changes
	* in the future, some backwards compatability will be maintained.
	* 
	*/
	
	std::stringstream dataToSerialize;

	dataToSerialize << this->cpuInternals.getSerialFormat() << '\n';
	dataToSerialize << this->ppuInternals.getSerialFormat() << '\n';

	return dataToSerialize.str();
}
