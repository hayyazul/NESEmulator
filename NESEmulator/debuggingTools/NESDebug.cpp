#include "NESDebug.h"
#include "debugDatabus.h"
#include "CPUAnalyzer.h"
#include <minmax.h>

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
