// Provides a controlled way of connecting two 
#pragma once

#include "../databus/nesDatabus.h"
#include <string>

namespace DMACycles {
	enum CycleType {
		READ,
		WRITE
	};
};

/*#include "directMemoryAccess.h"

OAMDMAUnit::OAMDMAUnit() : address(0), endAddress(0x100), OAMDataToTransfer(0), readOrWrite(DMACycles::CycleType::READ), databus(nullptr) {}
OAMDMAUnit::OAMDMAUnit(DataBus* CPUDatabus) : address(0), endAddress(0x100), OAMDataToTransfer(0), readOrWrite(DMACycles::CycleType::READ), databus(CPUDatabus) {}

OAMDMAUnit::~OAMDMAUnit() {}

bool OAMDMAUnit::performDMACycle(bool cpuCycleType) {
	// 512 or 513 cycles should be executed here (1 cycle, the DMA halt cycle, has already been executed by the CPU).
	// Ordering will be based on this fact, so when I say "first" here, it is actually the second cycle the CPU has been halted.

	const uint16_t OAMDATA = 0x2004;  // PPU register for OAMDATA.

	// Write/read
	if (this->readOrWrite && cpuCycleType) {  // Write
		// Write to OAM via OAMDATA. The address in the PPU for OAMDATA increments automatically when OAMDATA is written to.
		this->databus->write(OAMDATA, this->OAMDataToTransfer);
		this->readOrWrite = DMACycles::CycleType::READ;
	} else if (!this->readOrWrite && !cpuCycleType) {  // Read
		this->OAMDataToTransfer = this->databus->read(this->address);
		this->readOrWrite = DMACycles::CycleType::WRITE;
		++this->address;
	}

	// Lastly, check if we are done w/ the DMA (the last cycle was write and we are on the end address; note that if the last cycle was write, the read/write flag is now read). 
	bool DMAFinished = !this->readOrWrite && this->address == this->endAddress;
	return DMAFinished;
}

void OAMDMAUnit::setPage(uint8_t page) {
	this->address = (uint16_t)page << 8;  // We offset the page
	this->endAddress = this->address += 0x100;  // We want to stop on address 0x(NN + 1)00 exclusive. 
}

void OAMDMAUnit::attachDatabus(DataBus* CPUDatabus) {
	this->databus = CPUDatabus;
}
*/

// Contains the internal state of a OAMDMA unit; excludes the NESDatabus pointer.
struct OAMDMAInternals {
	DMACycles::CycleType readOrWrite;  
	uint16_t endAddress;
	uint16_t address;  
	uint8_t OAMDataToTransfer;  

	OAMDMAInternals();
	OAMDMAInternals(DMACycles::CycleType rOrW, uint16_t endAddr, uint16_t addr, uint8_t dataToTransfer);
	~OAMDMAInternals();

	// Gets the serial format of the unit (used in serializing save states).
	std::string getSerialFormat() const;
};

// A DMA unit for the PPU. First, it connects to the same databus the CPU uses, then, when the NES needs to use this, it calls the DMA unit to perform its action.
class OAMDMAUnit {
public:
	OAMDMAUnit();
	OAMDMAUnit(NESDatabus* CPUDatabus);
	~OAMDMAUnit();

	bool performDMACycle(bool cpuCycleType);  // Should be called every CPU cycle, passing in a bool indicating whether it is on the get (false) or put (true) cycle. See implementation for comments on implementation detail.
	// Returns false when the unit is not performing any more DMA cycles. 
	// NOTE: I might create an enum indicating the result of a DMA cycle (since some of them do nothing and thus can allow the other DMA unit to perform an action).

	void setPage(uint8_t page);  // Sets the page the OAMDMA unit will copy data over from.

	// Things needed to attach
	void attachDatabus(NESDatabus* CPUDatabus);  // Attached a databus to this unit; the databus you attach should be the same one used by the CPU.

	// Copies the values inside a given dma unit to this one EXCEPT the pointer to the databus.
	OAMDMAUnit& operator=(const OAMDMAUnit& otherDMAUnit);

	// Returns a copy of the internal state of this unit.
	OAMDMAInternals getInternals() const;

private:
	DMACycles::CycleType readOrWrite;  // Whether this DMA unit needs to read (false) or write (true).
	uint16_t endAddress;  // The address to stop transfering data on.  
	uint16_t address;  // The address the DMA unit is currently on and planning to read.
	uint8_t OAMDataToTransfer;  // When on a cycle that stores a byte, that byte is stored in this variable. It will later be passed to a write on 0x2004 (OAMDATA).
	NESDatabus* databus;  // This should be the same databus the CPU uses.

};