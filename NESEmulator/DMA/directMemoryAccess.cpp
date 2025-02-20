#include "directMemoryAccess.h"
#include <sstream>

OAMDMAUnit::OAMDMAUnit() : address(0), endAddress(0x100), OAMDataToTransfer(0), readOrWrite(DMACycles::CycleType::READ), databus(nullptr) {}
OAMDMAUnit::OAMDMAUnit(NESDatabus* CPUDatabus) : address(0), endAddress(0x100), OAMDataToTransfer(0), readOrWrite(DMACycles::CycleType::READ), databus(CPUDatabus) {}

OAMDMAUnit::~OAMDMAUnit() {}

bool OAMDMAUnit::performDMACycle(bool cpuCycleType) {
	// 512 or 513 cycles should be executed here (1 cycle, the DMA halt cycle, has already been executed by the CPU).
	// Ordering will be based on this fact, so when I say "first" here, it is actually the second cycle the CPU has been halted.

	const uint16_t OAMDATA = 0x2004;  // PPU register for OAMDATA.

	// Write/read
	if (this->readOrWrite == DMACycles::CycleType::WRITE && cpuCycleType) {  // Write (put, true)
		// Write to OAM via OAMDATA. The address in the PPU for OAMDATA increments automatically when OAMDATA is written to.

		if (this->address == 0x2a0) {
			int _ = 0;
		}
		if (this->address == 0x2ff) {
			int _ = 0;
		}

		this->databus->write(OAMDATA, this->OAMDataToTransfer);
		this->readOrWrite = DMACycles::CycleType::READ;
	}
	else if (this->readOrWrite == DMACycles::CycleType::READ && !cpuCycleType) {  // Read (get, false)
		this->OAMDataToTransfer = this->databus->read(this->address);
		if (this->OAMDataToTransfer != 0x7f && this->address == 0x200) {  // NOTE: This failed to catch the "blink" of the sprite...
			int _ = 0;
		}
		this->readOrWrite = DMACycles::CycleType::WRITE;
		++this->address;
	}

	// Lastly, check if we are done w/ the DMA (the last cycle was write and we are on the end address; note that if the last cycle was write, the read/write flag is now read). 
	bool continueDMA = this->readOrWrite || this->address != this->endAddress;
	return continueDMA;
}

void OAMDMAUnit::setPage(uint8_t page) {
	this->address = (uint16_t)page << 8;  // We offset the page
	this->endAddress = this->address + 0x100;  // We want to stop on address 0x(NN + 1)00 exclusive. 
}

void OAMDMAUnit::attachDatabus(NESDatabus* CPUDatabus) {
	this->databus = CPUDatabus;
}

OAMDMAUnit& OAMDMAUnit::operator=(const OAMDMAUnit& otherDMAUnit) {
	this->readOrWrite = otherDMAUnit.readOrWrite;
	this->endAddress = otherDMAUnit.endAddress;
	this->address = otherDMAUnit.address;
	this->OAMDataToTransfer = otherDMAUnit.OAMDataToTransfer;

	return *this;
}

OAMDMAInternals OAMDMAUnit::getInternals() const {
	return OAMDMAInternals(this->readOrWrite, this->endAddress, this->address, this->OAMDataToTransfer);
}

OAMDMAInternals::OAMDMAInternals() {}

OAMDMAInternals::OAMDMAInternals(DMACycles::CycleType rOrW, uint16_t endAddr, uint16_t addr, uint8_t dataToTransfer) :
	readOrWrite(rOrW),
	endAddress(endAddr),
	address(addr),
	OAMDataToTransfer(dataToTransfer)
{}

OAMDMAInternals::~OAMDMAInternals()
{
}

std::string OAMDMAInternals::getSerialFormat() const {
	std::stringstream preSerialStr;
	preSerialStr << "RORW: " << (int)this->readOrWrite << '\n';
	preSerialStr << "ENDADDR: " << (int)this->endAddress << '\n';
	preSerialStr << "ADDR: " << (int)this->address << '\n';
	preSerialStr << "DATATOTRANS: " << (int)this->OAMDataToTransfer << '\n';

	return preSerialStr.str();
}
