// parseNESFiles.h - A set of functions to decode iNES files and put them in a formatted struct.
#pragma once

#include "../memory/memory.h"
#include <stdint.h>
#include <vector>
#include <fstream>
#include <iostream>
#include <iomanip>
#include <array>
#include <set> 

enum Result {
	FAILURE,  // 0: When in the process of reading the file, some catastrophic failure prevents a full read. Used in general cases when the cause can't be figured out.
	SUCCESS,  // 1: When everything has been successfully been read.
	SIZE_MISTMATCH,  // 2: When the program or character data does not match up w/ the data size indicated in the header.
	BAD_HEADER,  // 3: When the header fails to match up with "NES(0x1a)"
	UNRECOGNIZED_MAPPER,  // 4: When the mapper fails to match up with a known or implemented one.
	CANT_OPEN_FILE  // 5: When opening the file fails.
};

const unsigned int PRG_DATA_CHUNK_SIZE = 0x4000;
const unsigned int CHR_DATA_CHUNK_SIZE = 0x2000;
const unsigned int HEADER_SIZE = 0x10;  // The header is 16 bytes long.

const std::set<uint8_t> IMPLEMENTED_MAPPERS = { 0 };

// This struct contains the file's mapperID and program and character data. Supports only iNES 1.0 type files.
struct NESFileData {
	uint8_t mapperID;
	unsigned int programDataSize = -1;
	unsigned int characterDataSize = -1;

	// Note: a vector in this context is an address at the end of addressable memory used to indicate where to initialize the program counter and the like.
	uint16_t NMIVector[2], RESETVector[2], IRQandBRKVector[2];  // Memory addresses at the very end of the program data pointing where to start execution.

	std::vector<uint8_t> programData;
	std::vector<uint8_t> characterData;
	Memory* CHRDATA;  // NOTE: I am putting the CHRDATA here because NES cartridges store their CHRDATA on the cartridge.
	// I do not know if doing this code-wise is the best approach, but I am doing it for now to see how it goes.
	// Alternatively, I coould create a pointer to CHRDATA in NESEmulator.

	NESFileData() : CHRDATA(nullptr) {
		this->CHRDATA = new Memory(0x2000);  // TODO: Implement bank-switching for CHRDATA.
	};
	~NESFileData() {};

	// Checks if the size of the program and character data correspond to the program and character size indicated in the header.
	bool assertValidity() const {  // Checks if the internal data is invalid. Right now it is not a sufficient test, but it does do a basic sanity check.
		if (programDataSize == programData.size() && characterDataSize == characterData.size()) {
			return true;
		}
		return false;
	}
	void findVectors() {
		if (this->assertValidity()) {  // This program assumes programData.size() == programDataSize, so check if this passes that check first.
			unsigned int lowerBytes[3];
			unsigned int upperBytes[3];

			for (unsigned int i = 0; i < 6; ++i) {  // Remember that the NES is little endian, so the lower bytes (0, 2, 4) come before the upper ones (1, 3, 5)
				if (!(i % 2)) {
					lowerBytes[i / 2] = this->programData.at(this->programDataSize - 6 + i);
				} else {
					upperBytes[(i - 1) / 2] = this->programData.at(this->programDataSize - 6 + i);
				}
			}

			this->NMIVector[0] = lowerBytes[0];
			this->NMIVector[1] = upperBytes[0];
			this->RESETVector[0] = lowerBytes[1];
			this->RESETVector[1] = upperBytes[1];
			this->IRQandBRKVector[0] = lowerBytes[2];
			this->IRQandBRKVector[1] = upperBytes[2];
		}
	}
};

Result parseiNESFile(const char* filename, NESFileData& gameData);
