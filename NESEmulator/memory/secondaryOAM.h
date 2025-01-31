#pragma once

#include "memory.h"

// An interface for secondary OAM, which is fixed at 256 bytes; the difference between this and a 
// regular memory module is that it is able to disable writes 
// (so calling the setByte method of this wouldn't cause any errors, but it won't do anything.)
class SecondaryOAM : public Memory {
public:
	SecondaryOAM();
	~SecondaryOAM();

	// NOTE: Might make private. This also does not change what bytes are free or not.
	uint8_t setByte(uint16_t address, uint8_t value) override;

	// Sets the first free byte to the given value, marks it as unfree. Also locks further writing when all free bytes are used.
	uint8_t setFreeByte(uint8_t value);

	// Frees all bytes, allowing them to be overwritten w/ setFreeByte
	void freeAllBytes();

	// Sets the state of writeEnabled while returning the old value; 
	bool setWriteState(bool writeState);

	// Returns the current writeEnabled state.
	bool getWriteState() const;

private:
	bool writeEnabled;
	uint8_t freeByteIdx;  // An index pointing to the current free byte.

};