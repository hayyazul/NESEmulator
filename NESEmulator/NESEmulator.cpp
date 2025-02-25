#include "NESEmulator.h"

NES::NES() : 
	memory(nullptr),
	ram(nullptr),
	databus(nullptr),
	CPU(nullptr),
	VRAM(nullptr),
	ppu(nullptr),
	DMAUnit(nullptr), 
	haltCPUOAM(false), 
	scheduleHalt(false), 
	totalMachineCycles(0) {

	/*
	this->memory = new Memory(0x10000);  // 0x10000 is the size of the addressing space.
	this->ram = new RAM();
	this->databus = new NESDatabus(this->memory);
	this->CPU = new _6502_CPU(this->databus);

	this->VRAM = new Memory(0x800);
	this->ppu = new PPU(this->VRAM, nullptr);
	
	this->DMAUnit.attachDatabus(this->databus);
	*/
}

NES::NES(NESDatabus* databus, _6502_CPU* CPU, RAM* ram, Memory* vram, PPU* ppu) : DMAUnit(databus), haltCPUOAM(false), scheduleHalt(false), totalMachineCycles(0) {
	this->ram = ram;
	this->ppu = ppu;
	this->VRAM = vram;
	this->ppu->attachVRAM(vram);
	this->databus = databus;
	this->databus->attach(this->memory);
	this->databus->attach(this->ram);
	this->databus->attach(this->ppu);
	this->CPU = CPU;
	this->CPU->attach(this->databus);
}

NES::~NES() {}

void NES::attachCPU(_6502_CPU* CPU) {
	this->CPU = CPU;
	this->CPU->attach(this->databus);
}

void NES::attachRAM(RAM* ram) {
	this->ram = ram;
	if (this->databus != nullptr) {
		this->databus->attach(ram);
	}
}

void NES::attachCartridgeMemory(Memory* memory) {
	this->memory = memory;
	if (this->databus != nullptr) {
		this->databus->attach(memory);
	}
}

void NES::attachDataBus(NESDatabus* databus) {
	this->databus = databus;
	this->databus->attach(this->memory);
	this->databus->attach(this->ppu);
	if (this->CPU != nullptr) {
		this->CPU->attach(this->databus);
	}
	this->databus->attach(this->ram);
	this->DMAUnit.attachDatabus(this->databus);
}

void NES::attachPPU(PPU* ppu) {
	this->ppu = ppu;
	this->ppu->attachVRAM(this->VRAM);
	if (this->databus != nullptr) {
		this->databus->attach(ppu);
	}
}

void NES::attachVRAM(Memory* vram) {
	this->VRAM = vram;
	if (this->ppu != nullptr) {
		this->ppu->attachVRAM(vram);
	}
}

void NES::powerOn() {
	// the CPU takes 7 CPU cycles to power on and 25 PPU cycles in the mean time.
	this->totalMachineCycles += 25;
	for (int i = 0; i < 25; ++i) {
		this->ppu->executePPUCycle();
	}
	this->CPU->powerOn();
}

void NES::reset() {
	this->CPU->reset();
}

void NES::loadROM(const char* fileName) {  // Remember to reset the NES after loading a ROM.
 	NESFileData NESFile;
	Result result = parseiNESFile(fileName, NESFile);

	if (result == SUCCESS) {
		this->loadData(NESFile);
	} else {
		std::cout << "iNES file parsing failed: " << result << std::endl;
	}
}

NESCycleOutcomes NES::performCPUCycle() {
	NESCycleOutcomes nesResult = PPU_CYCLE;
	CPUCycleOutcomes cpuResult = PASS;

	// Execute a CPU cycle.
	cpuResult = this->CPU->executeCycle(this->haltCPUOAM);

	switch (cpuResult) {
	case(INSTRUCTION_EXECUTED): 
		nesResult = INSTRUCTION_AND_PPU_CYCLE;
		break;
	case(FAIL):
		nesResult = FAIL_CYCLE;
		break;
	default:
		nesResult = BOTH_CYCLE;
		break;
	}
	
	if (this->haltCPUOAM) {  // If it is halted, then perform the appropriate DMA action. NOTE: I will need to later expand this to work with the APU's DMA.
		this->haltCPUOAM = this->DMAUnit.performDMACycle(this->CPU->getCycleType());
	}

	// Then check if we have scheduled an OAM DMA.
	if (this->scheduleHalt) {
		this->haltCPUOAM = true;
		this->scheduleHalt = false;
	}

	// Lastly, alternate the CPU cycle type.
	this->CPU->alternateCycle();

	return nesResult;
}

void NES::performPPUCycle() {
	// Performing the PPU cycle
  	this->ppu->executePPUCycle();
	this->CPU->requestNMI(this->ppu->requestingNMI());
	
	if (this->ppu->reqeuestingDMA()) {
		this->DMAUnit.setPage(this->ppu->getDMAPage());
		// The check for the CPU not already being halted is to ensure we don't reschedule a halt when it has already happened.
		if (!this->haltCPUOAM) {
			this->scheduleHalt = true;  // We want to execute one more CPU cycle before halting it, so we schedule it.
		}
	}
}

NESCycleOutcomes NES::executeMachineCycle() {
	// NOTE: The master clock on the real NES runs 3x faster, so 1 PPU cycle every 3 master clock cycles. 
	// This detail won't affect the behavior of this emulator, so we just make 1 machine cycle equal to 1 ppu cycle.
	NESCycleOutcomes nesResult = PPU_CYCLE;
	
	// Performing the CPU cycle;
	if (this->totalMachineCycles % 3 == 0) {
		nesResult = this->performCPUCycle();
	}

	this->performPPUCycle();

	++this->totalMachineCycles; 
	
	return nesResult;
}

void NES::loadData(NESFileData file) {
	// Then we load in ROM data.

	// Note: There is a special case regarding PRG ROM; if its size is 0x4000 (16K), we will duplicate it.
	// This is because we start loading in from address 0x8000, and at 16K we will only reach 0xc000. By duplicatin
	bool duplicateData = false;
	if (file.programDataSize == 0x4000) {
		duplicateData = true;
		file.programDataSize *= 2;
		std::vector<uint8_t> tempProgramData;
		// Duplicating data.
		for (int i = 0; i < 2; ++i) {
			for (uint8_t byte : file.programData) {
				tempProgramData.push_back(byte);
			}
		}
		file.programData = tempProgramData;
	}

	uint16_t j = 0;
	for (uint32_t i = this->CART_ROM_START_ADDR; i <= 0xffff; ++i) {
		if (j < file.programDataSize) {
			this->memory->setByte(i, file.programData[j]);
		} //else if (j - file.programDataSize < file.characterDataSize) {  // NOTE: I am confused on where to put CHR ROM, so for now I don't put it in at all.
			//this->databus.write(i, file.programData[j - file.programDataSize]);
		//}
		++j;
	}

	// Attaching CHRDATA to the PPU.
	ppu->attachCHRDATA(file.CHRDATA);
}
