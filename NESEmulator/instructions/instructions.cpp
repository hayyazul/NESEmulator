#include "instructions.h"
#include "../6502Chip/CPU.h"

namespace addressingOperations {
    uint8_t immediate(DataBus& databus, Registers& registers) {
        // The value after the opcode is treated as data and not an address.
        return databus.read(registers.PC + 1);  
    }

    uint8_t implicit(DataBus& databus, Registers& registers) {
        // Implicit instructions do not use any value from RAM. This 0 will go unused.
        return 0;  
    }

    uint8_t accumulator(DataBus& databus, Registers& registers) {
        // Instructions with this addressing mode operate on the accumulator; not on RAM.
        return registers.A;
    }

    uint8_t zeropage(DataBus& databus, Registers& registers) {
        // Indexes 0x00LL; zeropage takes fewer cycles than other addressing modes.
        uint16_t address = static_cast<uint16_t>(databus.read(registers.PC + 1));
        return databus.read(address);
    }
    uint8_t zeropageX(DataBus& databus, Registers& registers) {
        // Indexes 0x00LL + X; zeropage takes fewer cycles than other addressing modes.
        uint16_t address = static_cast<uint16_t>(databus.read(registers.PC + 1));
        return databus.read(address + registers.X);
    }
    uint8_t zeropageY(DataBus& databus, Registers& registers) {
        // Indexes 0x00LL + Y; zeropage takes fewer cycles than other addressing modes.
        uint16_t address = static_cast<uint16_t>(databus.read(registers.PC + 1));
        return databus.read(address + registers.Y);
    }
    uint8_t absolute(DataBus& databus, Registers& registers) {
        // The NES is a little-endian machine, meaning the FIRST byte read takes up the LOWER 8 bits.
        // So if in memory we had these values: 8f 30
        // We would return a memory address of 0x308f
        // lb = lower byte; ub = upper byte; addr = address
        uint16_t lbOfAddr = static_cast<uint16_t>(databus.read(registers.PC + 1));
        uint16_t ubOfAddr = static_cast<uint16_t>(databus.read(registers.PC + 2)) << 8;
        return databus.read(lbOfAddr + ubOfAddr);
    }
    uint8_t absoluteX(DataBus& databus, Registers& registers) {
        // The NES is a little-endian machine, meaning the FIRST byte read takes up the LOWER 8 bits.
        // So if in memory we had these values: 8f 30
        // We would return a memory address of 0x308f + X 
        // lb = lower byte; ub = upper byte; addr = address
        uint16_t lbOfAddr = static_cast<uint16_t>(databus.read(registers.PC + 1));
        uint16_t ubOfAddr = static_cast<uint16_t>(databus.read(registers.PC + 2)) << 8;
        return databus.read(lbOfAddr + ubOfAddr + registers.X);
    }
    uint8_t absoluteY(DataBus& databus, Registers& registers) {
        // The NES is a little-endian machine, meaning the FIRST byte read takes up the LOWER 8 bits.
        // So if in memory we had these values: 8f 30
        // We would return a memory address of 0x308f + Y
        // lb = lower byte; ub = upper byte; addr = address
        uint16_t lbOfAddr = static_cast<uint16_t>(databus.read(registers.PC + 1));
        uint16_t ubOfAddr = static_cast<uint16_t>(databus.read(registers.PC + 2)) << 8;
        return databus.read(lbOfAddr + ubOfAddr + registers.Y);
    }

    // Works similar to pointers to addresses. 
    // It first goes to the given memory address, looks at the byte and the byte 
    // of the next address, uses those two bytes to make a new address 
    // which it gets the value of.
    uint8_t indirect(DataBus& databus, Registers& registers) {
        // lb = lower byte; ub = upper byte; addr = address

        // First, we get the values of the next 2 bytes (the address contained in the pointer)
        uint16_t lbOfPtrAddr = static_cast<uint16_t>(databus.read(registers.PC + 1));
        uint16_t ubOfPtrAddr = static_cast<uint16_t>(databus.read(registers.PC + 2)) << 8;
        // We then find the address of the data located at the address given by the pointer.
        uint16_t lbOfAddr = databus.read(lbOfPtrAddr + ubOfPtrAddr);
        uint16_t ubOfAddr = databus.read(lbOfPtrAddr + ubOfPtrAddr + 1) << 8;

        // Finally we get the data.
        return databus.read(lbOfAddr + ubOfAddr);

    }
    uint8_t indirectX(DataBus& databus, Registers& registers) {
        // lb = lower byte; ub = upper byte; addr = address
        // This addressing mode is zeropage.

        // First, we get the values of the pointer.
        uint16_t ptrAddr = static_cast<uint16_t>(databus.read(registers.PC + 1));
        // We then find the address located at the address given by the pointer + the X register.
        uint16_t addr = databus.read(ptrAddr + registers.X);

        // Then we get the data at this address.
        return databus.read(addr);
    } 
    uint8_t indirectY(DataBus& databus, Registers& registers) {
        // lb = lower byte; ub = upper byte; addr = address
        // This addressing mode is zeropage.

        // First, we get the values of the pointer.
        uint16_t ptrAddr = static_cast<uint16_t>(databus.read(registers.PC + 1));
        // We then find the address located at the address given by the pointer.
        uint16_t addr = databus.read(ptrAddr);

        // Finally
        return databus.read(addr + registers.X);
    }
}

/* void ADC
    Adds the given data to the accumulator.

    Flags Affected:
     - C: if overflow in bit 7.
     - Z: if the accumulator = 0.
     - V: if the sign bit is incorrect
     - N: if bit 7 is set.

    Addressing Modes:
     - Immediate: 2 Bytes, 2 Cycles
     - Zero Page: $65, 2 Bytes, 3 Cycles
     - Zero Page,X: $75, 2 Bytes, 4 Cycles
     - Absolute: $6D, 3 Bytes, 4 Cycles
     - Absolute,X: $7D, 3 Bytes, 4 Cycles (+1 if page crossed)
     - Absolute,Y: $79, 3 Bytes, 4 Cycles (+1 if page crossed)
     - (Indirect,X): $61, 2 Bytes, 6 Cycles
     - (Indirect),Y: $71, 2 Bytes, 5 Cycles (+1 if page crossed)

    TODO:
     Implement C and V flaggings.
*/
namespace operations {
    void ADC(Registers& registers, uint8_t data) {
        registers.A += data;
        if (0b10000000 & registers.A) {
            registers.setStatus('N', true);
        }
        else {
            registers.setStatus('N', false);
        }

        if (registers.A == 0) {
            registers.setStatus('Z', true);
        }
        else {
            registers.setStatus('Z', false);
        }
    }
}