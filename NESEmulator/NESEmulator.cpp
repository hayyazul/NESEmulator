#include "NESEmulator.h"

NES::NES() {  // Not recommended to initialize w/ this; this will cause a memory leak later. NOTE: Might just make these nullptrs.
	this->memory = new Memory(0x10000);  // 0x10000 is the size of the addressing space.
	this->ram = new RAM();
	this->databus = new NESDatabus(this->memory);
	this->CPU = new _6502_CPU(this->databus);

	this->VRAM = new Memory(0x800);
	this->ppu = new PPU(this->VRAM, nullptr);
	
	this->CPU->powerOn();
}

NES::NES(NESDatabus* databus, _6502_CPU* CPU, RAM* ram, Memory* vram, PPU* ppu) {
	this->ram = ram;
	this->ppu = ppu;
	this->VRAM = vram;
	this->ppu->attach(vram);
	this->databus = databus;
	this->databus->attach(this->memory);
	this->databus->attach(this->ram);
	this->databus->attach(this->ppu);
	this->CPU = CPU;
	this->CPU->attach(this->databus);
}

NES::~NES() {}

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
	this->databus->attach(this->memory);
}

void NES::attachPPU(PPU* ppu) {
	this->ppu = ppu;
	this->databus->attach(ppu);
}

void NES::attachVRAM(Memory* vram) {
	this->VRAM = vram;
	this->ppu->attach(vram);
}

void NES::powerOn() {
	// the CPU takes 7 CPU cycles to power on, so perform 21 machine cycles in the mean time.
	this->totalMachineCycles += 21;
	for (int i = 0; i < 21; ++i) {
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

NESCycleOutcomes NES::executeMachineCycle() {
	// NOTE: The master clock on the real NES runs 3x faster, so 1 PPU cycle every 3 master clock cycles. 
	// This detail won't affect the behavior of this emulator, so we just make 1 machine cycle equal to 1 ppu cycle.
	NESCycleOutcomes nesResult = PPU_CYCLE;
	CPUCycleOutcomes cpuResult = FAIL;
	if (this->totalMachineCycles % 3 == 0) {
		cpuResult = this->CPU->executeCycle();

		if (cpuResult == INSTRUCTION_EXECUTED) {
			nesResult = INSTRUCTION_AND_PPU_CYCLE;
		} else if (cpuResult == FAIL) {
			nesResult = FAIL_CYCLE;
		} else {
			nesResult = BOTH_CYCLE;
		} 
	}
	this->ppu->executePPUCycle();
	
	// NMI request logic; this is done when the Vertical-blanking interval is reached.
	if (this->ppu->requestingNMI()) {
		this->CPU->requestNMI();
	}

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
}
