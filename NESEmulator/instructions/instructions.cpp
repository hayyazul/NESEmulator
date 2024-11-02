#include "instructions.h"
#include "../6502Chip/CPU.h"

namespace flagOps {
    // a = accumulatr; c = carry flag.
    bool isSignBitIncorrect(uint8_t aBefore, uint8_t sum, uint8_t data) {
        // While xor affects all bits, we only care about bit 7 (the signed bit).
        // The inner parentheses check if the sign has changed (via xor-ing).
        // The & between the two check if the sign has changed for both the accumulator and the data value.
        // This also has the effect of checking if the accumulator and the data were of the same sign.
        // The final & with 0b10000000 is to filter out all bits but the 7th.
        return ((aBefore ^ sum) & (data ^ sum)) & 0b10000000;
    }

    bool isBit7Set(uint8_t byte) {
        return 0b10000000 & byte;
    }

    bool isOverflow(uint8_t a, uint8_t data, bool c) {
        // Note that overflow occurs when: a + data + c > 255; a > 255 - (data + c)
        // Since attempting to add the two would result in an overflow
        // Into a value under 255, we subtract instead to avoid that.
        // There is a chance for data + c to overflow itself; catch this case and return
        // the overflow.
        const uint8_t MAX_UINT8 = 255;
        return (data > MAX_UINT8 - c) || (a > MAX_UINT8 - (data + c));
    }

    bool isUnderflow(uint8_t a, uint8_t data, bool c) {
        // a - data - (1 - c) < 0; a - 1 + c < data
        // Again, we can't compare directly as an underflow will occur.
        // So we rearrange the inequality.
        // First, check for the edge case where a is less than 1 - c.
        return (a < 1 - c) || (a - 1 + c < data);
    }
}

namespace dataAddrOp {
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
    uint8_t relative(DataBus& databus, Registers& registers) {
        // The offset is a signed byte
        int8_t offset = databus.read(registers.PC + 1);
        return databus.read(registers.PC + offset);
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

// TODO: move comments to header file.
namespace ops {
    /* void ADC
    Adds the given data and the carry to the accumulator.

    Flags Affected:
     - C: if overflow in bit 7.
     - Z: if the accumulator = 0.
     - V: if the sign bit is incorrect;
     i.e. if adding two unsigned numbers results in a negative number
     - N: if bit 7 is set.
    */
    void ADC(Registers& registers, uint8_t data) {
        uint8_t accumulatorPreOp = registers.A;
        registers.A += data + registers.getStatus('C');

        registers.setStatus('N', 0b10000000 & registers.A);
        registers.setStatus('Z', registers.A == 0);
        registers.setStatus('V', flagOps::isSignBitIncorrect(accumulatorPreOp, registers.A, data));
        // This one checks if we overflowed the accumulator
        registers.setStatus('C', flagOps::isOverflow(accumulatorPreOp, data, registers.getStatus('C')));
    }
    /* void AND
    Performs bitwise AND on accumulator and data.

    Flags Affected:
     - Z: set if the accumulator = 0.
     - N: set if bit 7 is set.
    */
    void AND(Registers& registers, uint8_t data) {
        registers.A &= data;
        registers.setStatus('N', flagOps::isBit7Set(registers.A));
        registers.setStatus('Z', registers.A == 0);
    }
    /* void ASL
    Performs a shift left.
    
    Flags Affected:
     - C: set to value of old bit 7
     - Z: set if A = 0
     - N: set to value of new bit 7
     */
    void ASL(Registers& registers, uint8_t data) {
        registers.setStatus('C', flagOps::isBit7Set(registers.A));
        registers.A = data << 1;
        registers.setStatus('N', flagOps::isBit7Set(registers.A));
        registers.setStatus('Z', registers.A == 0);
    }
    /* void BCC
    Performs a branch if the carry flag is 0.

    Flags Affected:
        None

    */
    void BCC(Registers& registers, uint8_t data) {
        int8_t offset = data;  // Treat the data as signed.
        if (!registers.getStatus('C')) {
            registers.PC += offset;
        }
    }

    /* void BCS
    Performs a branch if the carry flag is 1.

    Flags Affected:
        None
    */
    void BCS(Registers& registers, uint8_t data) {
        int8_t offset = data;  // Treat the data as signed.
        if (registers.getStatus('C')) {
            registers.PC += offset;
        }
    }
    /* void BEQ
    Performs a branch if the zero flag is 1.
    The zero flag should have been set by another opcode [TODO: name it] before.

    Flags Affected:
        None

    */
    void BEQ(Registers& registers, uint8_t data) {
        int8_t offset = data;  // Treat the data as signed.
        if (registers.getStatus('Z')) {
            registers.PC += offset;
        }
    }
    /* void BIT
    Test if some bits are set in a memory location.
    Does a bitwise AND with the accumulator and the data,
        
    Flags Affected:
        None
    */
    void BIT(Registers& registers, uint8_t data) {
        int8_t offset = data;  // Treat the data as signed.
        if (registers.getStatus('C')) {
            registers.PC += offset;
        }
    }
    /* void LDA
    Loads the data at a memory location into the accumulator

    Flags Affected:
     - Z: set to 1 if A = 0.
     - N: set to 1 if bit 7 is set.
    */
    void LDA(Registers& registers, uint8_t data) {
        registers.A = data;
    }
    /* void STA
    Stores the accumulator in a memory location.

    Flags Affected:
        None
    */
    void STA(Registers& registers, DataBus& databus, uint16_t data) {
        databus.write(data, registers.A);
    }
}

namespace addr16bitOp {
    uint16_t zeropage(DataBus& databus, Registers& registers) {
        return dataAddrOp::zeropage(databus, registers);
    }
}
