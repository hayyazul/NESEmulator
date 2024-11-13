#include "parseNESFiles.h"

Result parseiNESFile(const char* filename, NESFileData& gameData) {
	std::ifstream file{filename, std::ios_base::binary};
	
	if (!file) {
		return CANT_OPEN_FILE;
	}

	unsigned int i = 0;
	
	const uint8_t properHeader[4]{'N', 'E', 'S', 0x1a};
	uint8_t header[4];  // Contains first 4 bytes of the file; if they fail to match up with the above, return null.

	uint8_t data; 
	
	// We iterate through the file till we reach the end.
	while ((file >> std::noskipws >> data)) {
		// First we check the file signature.
		if (i < 4) {  // Getting the first 4 bytes.
			header[i] = data;
		} else if (i == 4) {  // Once we surpass 4 bytes, check if the signature is valid; if not we have failed.
			for (unsigned int j = 0; j < 4; ++j) {
				if (header[j] != properHeader[j]) {
					return BAD_HEADER;
				}
			}
		}

		// First, get the info from the header.
		switch(i) {
		case(0x4):  // Program ROM length is stored on the 5th byte.
			gameData.programDataSize = PRG_DATA_CHUNK_SIZE * data;
			break;
		case(0x5):  // Character ROM length is stored on the 6th byte.
			gameData.characterDataSize = CHR_DATA_CHUNK_SIZE * data;
			break;
		case(0x6):  // This is a flag on the 7th byte; the upper nybble contains the lower nybble of the mapper ID. For now, ignore everything else.
			gameData.mapperID = 0b1111 & (data >> 4);
			// Check if the mapperID we got is implemented.
			if (!IMPLEMENTED_MAPPERS.count(gameData.mapperID)) {
				return UNRECOGNIZED_MAPPER;
			}
			break;
		default:
			break;
		}

		if (i > 0xf) {
			// In the standard case, we check if the current byte index is within bounds as given by the program and character data.
			if (i < gameData.programDataSize + HEADER_SIZE) {  // Program data comes before character data, so if this is true, i being less than the address at where character data is stored is guaranteed.
				gameData.programData.push_back(data);
			}
			else if (i < gameData.programDataSize + gameData.characterDataSize + HEADER_SIZE) {
				gameData.characterData.push_back(data);
			}
		}

		++i;
	}

	// Check if the size we got from the header matches w/ the nummber of bytes we read.

	if (!gameData.assertValidity()) {
		return SIZE_MISTMATCH;
	}

	gameData.findVectors();
	return SUCCESS;
}