// parseNESFiles.h - A set of functions to decode iNES files and put them in a formatted struct.
#pragma once
#include <stdint.h>
#include <vector>
#include <fstream>
#include <iostream>
#include <iomanip>

enum Result {
	FAILURE,  // When in the process of reading the file, some catastrophic failure prevents a full read. Used in general cases when the cause can't be figured out.
	SUCCESS,  // When everything has been successfully been read.
	SIZE_MISTMATCH,  // When the program or character data does not match up w/ the data size indicated in the header.
	BAD_HEADER  // When the header fails to match up with "NES(0x1a)"
};

const unsigned int PRG_DATA_CHUNK_SIZE = 0x4000;
const unsigned int CHR_DATA_CHUNK_SIZE = 0x2000;
const unsigned int HEADER_SIZE = 0x10;  // The header is 16 bytes long.

// This struct contains the file's mapperID and program and character data.
struct NESFileData {
	uint16_t mapperID;
	unsigned int programDataSize = -1;
	unsigned int characterDataSize = -1;

	// Note: a vector in this context is an address at the end of addressable memory used to indicate where to initialize the program counter and the like.
	uint16_t NMIVector[2], RESETVector[2], IRQandBRKVector[2];  // Memory addresses at the very end of the program data pointing where to start execution.

	std::vector<uint8_t> programData;
	std::vector<uint8_t> characterData;

	NESFileData() {};
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
