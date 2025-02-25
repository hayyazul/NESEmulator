#include "NESDebug.h"
#include "debugDatabus.h"
#include "CPUAnalyzer.h"
#include <minmax.h>
#include <sstream>
#include <filesystem>

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

NESCycleOutcomes NESDebug::executeNMachineCycles(unsigned long long numCycles, bool CPUBased) {
	// First, validate the numCycles.
	NESCycleOutcomes result = PPU_CYCLE;
	if (numCycles <= 0) {
		result = FAIL_CYCLE;
	}
	else {
		// Then execute the specified number of times or until a failure occurs.
			if (CPUBased) {  // If CPU based, make sure to execute machine cycles until the number of cycles equals the desired amount.
				unsigned long long initCPUCount = this->getNumCPUCycles();
				unsigned long long currentCPUCount;
				do {
					result = this->executeMachineCycle();
					currentCPUCount = this->getNumCPUCycles();
					if (currentCPUCount - initCPUCount > numCycles) {
						int _ = 0;
						// This should never happen. If it does, it means we somehow iterated the number of CPU cycles by more than 1, which is not allowed.
					}
				} while(result != FAIL_CYCLE && (currentCPUCount - initCPUCount) != numCycles);
			} else {
				for (unsigned long long i = 0; i < numCycles && result != FAIL_CYCLE; ++i) {
					result = this->executeMachineCycle();
				
			}
		}
	}
	return result;
}

NESCycleOutcomes NESDebug::executeTillCycle(unsigned long long cycleCount, bool CPUBased) {
	// First, check if the cycle count was already exceeded.
	unsigned long long cyclesElapsed = CPUBased ? this->getNumCPUCycles() : this->getNumCycles();
	NESCycleOutcomes result;
	if (cyclesElapsed >= cycleCount) {
		result = FAIL_CYCLE;
	} else {
		// If not, execute the amount of cycles to reach the desired elapsed amount.
		result = this->executeNMachineCycles(cycleCount - cyclesElapsed, CPUBased);
	}

	return result;
}

bool NESDebug::frameFinished() const {
	bool atEndOfFrame = this->debugPPU.getPosition().inRange(239, 239, 256, 256);
	return atEndOfFrame;
}

unsigned long long NESDebug::getNumCycles() const {
	return this->totalMachineCycles;
}

unsigned long long NESDebug::getNumCPUCycles() const {
	return this->debugCPU.getNumCycles();
}

void NESDebug::getNESInternals(NESInternals& internals) {
	// We save the internals of the CPU, PPU, RAM, and VRAM.
	internals.cpuInternals = this->getCPUInternals();
	internals.ppuInternals = this->getPPUInternals();
	this->getRAM(internals.ram);
	internals.oamDMAUnit = this->getOAMDMAUnit();
	internals.haltCPUOAM = this->haltCPUOAM;
	internals.scheduleHalt = this->scheduleHalt;
	internals.totalMachineCycles = this->totalMachineCycles;
}


void NESDebug::loadInternals(NESInternals internals) {
	this->debugCPU.loadInternals(internals.cpuInternals);
	this->debugPPU.loadInternals(internals.ppuInternals);
	this->DMAUnit = internals.oamDMAUnit;
	*this->ram = internals.ram;
	this->haltCPUOAM = internals.haltCPUOAM;
	this->scheduleHalt = internals.scheduleHalt;
	this->totalMachineCycles = internals.totalMachineCycles;
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

NESInternals::NESInternals(std::filesystem::path filepath) {
	// First check if it is a valid file.
	std::string filename = filepath.filename().string();
	if (filename.find(".nesstate") == std::string::npos) {
		std::cout << "Invalid filepath; can not deserialize " << filename << std::endl;
		return;
	}

	// If it is, we begin deserializing. First we open the file.
	std::ifstream file{ filepath };
	if (!file.is_open()) {
		std::cout << "Unable to open file " << filename << "(Error: " << file.rdstate() << ")" << std::endl;
		return;
	}
	
	// The components in order of the file. Each component is seperated by an extra newline.
	enum ComponentOn {
		VERSION,
		NAME,
		CPU,
		PPU,
		DMA,
		RAM
	};

	// We begin with skipping the first line (the version id) and getting the name from the second, then chunking the various parts of the now deserialized data.
	std::stringstream cpuData;
	std::stringstream ppuData;
	std::stringstream DMAData;
	std::stringstream RAMData;
	ComponentOn component = VERSION;
	for (std::string line; std::getline(file, line);) {
		std::cout << line << '\n';
		// Different lines represent different parts of the file.
		switch (component) {
		case(VERSION): {
			component = NAME;
			break;
		}
		case(NAME): {
			this->name = line;
			component = CPU;
			break;
		}
		case(CPU): {
			if (line == "") {
				component = PPU;
				break;
			}
			cpuData << line << ' ';
			break;
		}
		case(PPU): {
			if (line == "") {
				component = DMA;
				break;
			}
			ppuData << line << ' ';
			break;
		}
		case(DMA): {
			if (line == "") {
				component = RAM;
				break;
			}
			DMAData << line << ' ';
			break;
		}
		case(RAM): {
			RAMData << line << ' ';
			break;
		}
		}
	}

	// TODO: COMPLETE
	// From then on, each component handles its own deserialization.
	this->cpuInternals.deserializeData(cpuData);
}

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

	// Gets the strings of the various internal structures and variables.
	dataToSerialize << this->name << '\n';
	dataToSerialize << this->cpuInternals.getSerialFormat() << '\n';
	dataToSerialize << this->ppuInternals.getSerialFormat() << '\n';
	dataToSerialize << this->oamDMAUnit.getInternals().getSerialFormat() << '\n';
	dataToSerialize << "RAM: " << this->ram.getDataAsStr();

	return dataToSerialize.str();
}

void displayMemDump(std::vector<uint8_t>& dump, uint16_t startAddr, uint16_t endAddr, unsigned int bytesPerRow) {
	unsigned int rowCount = ceil(static_cast<double>(endAddr - startAddr) / bytesPerRow);
	if (endAddr > startAddr) {
		for (unsigned int i = 0; i < rowCount; ++i) {
			displayMemDumpLine(dump, startAddr, endAddr, i, bytesPerRow);
		}
	}
}

void displayMemDumpLine(std::vector<uint8_t>& dump, uint16_t startAddr, uint16_t endAddr, unsigned int row, unsigned int bytesPerRow) {
	std::cout << displayHex(startAddr + row * bytesPerRow, 4) << " | ";
	unsigned int rowSize = min((endAddr - startAddr) - row * bytesPerRow, bytesPerRow);  // This gives you how many bytes are left to display in total (not just for this row).

	for (unsigned int i = 0; i <= rowSize; ++i) {
		std::cout << displayHex(dump.at(i + row * bytesPerRow), 2) << ' ';
	}
	std::cout << std::endl;
}
