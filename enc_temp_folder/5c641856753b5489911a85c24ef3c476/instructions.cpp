#include "instructions.h"
#include "../6502Chip/CPU.h"

namespace helperByteOps {
    // a = accumulatr; c = carry flag.
    bool isSignBitIncorrect(uint8_t aBefore, uint8_t sum, uint8_t data) {
        // While xor affects all bits, we only care about bit 7 (the signed bit).
        // The inner parentheses check if the sign has changed (via xor-ing).
        // The & between the two check if the sign has changed for both the accumulator and the data value.
        // This also has the effect of checking if the accumulator and the data were of the same sign.
        // The final & with 0b10000000 is to filter out all bits but the 7th.

        return ((aBefore ^ sum) & (data ^ sum) & 0b10000000);
    }

    bool isBit7Set(uint8_t byte) {
        return 0b10000000 & byte;
    }

    bool isBit0Set(uint8_t byte) {
        return 0b1 & byte;
    }

    // The CPU checks if a number is less than another by subtracting one from the other, then seeing if the negative bit is set.
    bool isLessThan(uint8_t a, uint8_t d) {
        uint8_t subResult = a - d;
        subResult &= 0b10000000;
        return subResult;
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
    
    bool crossedPgBoundary(uint16_t addr, int8_t addition) {
        int16_t firstByteOfAddr = addr & 0xff;
        return (firstByteOfAddr + addition > 0xff || firstByteOfAddr + addition < 0x00);  // Check if we went beyond 256 or 0 bytes in both directions.
    } 
    bool crossedPgBoundary(uint16_t addr, uint8_t addition) {
        uint8_t firstByteOfAddr = addr & 0xff;
        uint8_t addrSummed = firstByteOfAddr + addition;
        bool crossed = addrSummed < 0xff;  // Check if we went beyond 256.
        return crossed;
    }
}

namespace addrModes {
    uint16_t immediate(DataBus& dataBus, Registers& registers) {
        // The value after the opcode is treated as data and not an address; the address in this case is just the program counter iterated by 1.
        return registers.PC + 1;
    }

    uint16_t implicit(DataBus& dataBus, Registers& registers) {
        // Implicit instructions do not use any value from RAM. This 0 will go unused.
        return 0;  
    }

    uint16_t accumulator(DataBus& dataBus, Registers& registers) {
        // Instructions with this addressing mode operate on the accumulator; not on RAM. As a result, this 0 is unused.
        return 0;
    }

    uint16_t zeropage(DataBus& dataBus, Registers& registers) {
        // Indexes 0x00LL; zeropage takes fewer cycles than other addressing modes.
        uint16_t address = static_cast<uint16_t>(dataBus.read(registers.PC + 1));
        return address;
    }
    uint16_t zeropageX(DataBus& dataBus, Registers& registers) {
        // Indexes 0x00LL + X; zeropage takes fewer cycles than other addressing modes.
        uint16_t address = static_cast<uint16_t>(dataBus.read(registers.PC + 1));
        return address + registers.X;
    }
    uint16_t zeropageY(DataBus& dataBus, Registers& registers) {
        // Indexes 0x00LL + Y; zeropage takes fewer cycles than other addressing modes.
        uint16_t address = static_cast<uint16_t>(dataBus.read(registers.PC + 1));
        return address + registers.Y;
    }
    uint16_t relative(DataBus& dataBus, Registers& registers, bool& addCycles) {
        // The offset is a signed byte; return the address where the offset is located (the next byte).
        int8_t offset = dataBus.read(registers.PC + 1);  // Get the offset (this is always an offset as this addressing mode is used only by branch instructions).
        // Note: Since the CPU has read both the opcode and the sole operand, it is now "ahead" 2 memory
        // locations.
        addCycles = helperByteOps::crossedPgBoundary(registers.PC + 2, offset);
        return registers.PC + 1;
    }
    uint16_t absolute(DataBus& dataBus, Registers& registers) {
        // The NES is a little-endian machine, meaning the FIRST byte read takes up the LOWER 8 bits.
        // So if in memory we had these values: 8f 30
        // We would return a memory address of 0x308f
        // lb = lower byte; ub = upper byte; addr = address
        uint16_t lbOfAddr = static_cast<uint16_t>(dataBus.read(registers.PC + 1));
        uint16_t ubOfAddr = static_cast<uint16_t>(dataBus.read(registers.PC + 2)) << 8;
        return lbOfAddr + ubOfAddr;
    }
    uint16_t absoluteX(DataBus& dataBus, Registers& registers, bool& addCycles) {
        // The NES is a little-endian machine, meaning the FIRST byte read takes up the LOWER 8 bits.
        // So if in memory we had these values: 8f 30
        // We would return a memory address of 0x308f + X 
        // lb = lower byte; ub = upper byte; addr = address
        uint16_t lbOfAddr = static_cast<uint16_t>(dataBus.read(registers.PC + 1));
        uint16_t ubOfAddr = static_cast<uint16_t>(dataBus.read(registers.PC + 2)) << 8;
        addCycles = helperByteOps::crossedPgBoundary(lbOfAddr + ubOfAddr, registers.X);
        return lbOfAddr + ubOfAddr + registers.X;
    }
    uint16_t absoluteY(DataBus& dataBus, Registers& registers, bool& addCycles) {
        // The NES is a little-endian machine, meaning the FIRST byte read takes up the LOWER 8 bits.
        // So if in memory we had these values: 8f 30
        // We would return a memory address of 0x308f + Y
        // lb = lower byte; ub = upper byte; addr = address
        uint16_t lbOfAddr = static_cast<uint16_t>(dataBus.read(registers.PC + 1));
        uint16_t ubOfAddr = static_cast<uint16_t>(dataBus.read(registers.PC + 2)) << 8;
        addCycles = helperByteOps::crossedPgBoundary(lbOfAddr + ubOfAddr, registers.Y);
        return lbOfAddr + ubOfAddr + registers.Y;
    }

    // Works similar to pointers to addresses. 
    // It first goes to the given memory address, looks at the byte and the byte 
    // of the next address, uses those two bytes to make a new address 
    // which it gets the value of.
    uint16_t indirect(DataBus& dataBus, Registers& registers) {
        // lb = lower byte; ub = upper byte; addr = address

        // First, we get the values of the next 2 bytes (the address contained in the pointer)
        uint8_t lbOfPtrAddr = static_cast<uint16_t>(dataBus.read(registers.PC + 1));
        // Note: There is a bug in the 6502 that when dealing w/ an indirect JMP.
        // When the address is of the form xxFF, i.e. when the address is on a page boundary, the upper bit is mistakenly taken from
        // address xx00 instead of (xx + 1)00 as expected. This is emulated w/ (lbOfPtrAddr + 1); this statement should overflow into 00 when
        // it is equal to FF.
        uint16_t ubOfPtrAddr = static_cast<uint16_t>(dataBus.read(registers.PC + 2)) << 8;
        // We then find the address of the data located at the address given by the pointer.
        uint16_t lbOfAddr = dataBus.read(lbOfPtrAddr + ubOfPtrAddr);
        uint8_t lbOfPtrAddrOffset = (lbOfPtrAddr + 1);
        uint16_t ubOfAddr = dataBus.read(lbOfPtrAddrOffset + ubOfPtrAddr) << 8;

        return lbOfAddr + ubOfAddr;

    }
    uint16_t indirectX(DataBus& dataBus, Registers& registers) {
        // lb = lower byte; ub = upper byte; addr = address
        // This addressing mode is zeropage.

        // First, we get the value of the pointer (don't forget to add the X register)
        uint8_t ptrAddr = static_cast<uint16_t>(dataBus.read(registers.PC + 1)) + registers.X;
        // We then find the address located at the address given by the pointer.
        uint16_t addr = dataBus.read(ptrAddr);
        ptrAddr += 1;
        addr += dataBus.read(ptrAddr) << 8;
        // TODO: Fix this; it does not get the full address.

        return addr;
    } 
    uint16_t indirectY(DataBus& dataBus, Registers& registers, bool& addCycles) {
        // lb = lower byte; ub = upper byte; addr = address
        // This addressing mode is zeropage.

        // First, we get the values of the pointer.
        uint8_t ptrAddr = dataBus.read(registers.PC + 1);
        // We then find the address located at the address given by the pointer.
        uint16_t addr = dataBus.read(ptrAddr);
        ++ptrAddr;
        addr += dataBus.read(ptrAddr) << 8;
        addCycles = helperByteOps::crossedPgBoundary(addr, registers.Y);
        return addr + registers.Y;
    }
}

// TODO: move comments to header file.
// TODO: Use the new functions in databus to more easily communicate w/ the stack.
namespace ops {  // TODO: Fix operations which relied on the old system of fetching data directly.
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
        registers.setStatus('V', helperByteOps::isSignBitIncorrect(accumulatorPreOp, registers.A, data));
        // This one checks if we overflowed the accumulator
        registers.setStatus('C', helperByteOps::isOverflow(accumulatorPreOp, data, registers.getStatus('C')));
    }
    /* void AND
    Performs bitwise AND on accumulator and data.

    Flags Affected:
     - Z: set if the accumulator = 0.
     - N: set if bit 7 is set.
    */
    void AND(Registers& registers, uint8_t data) {
        registers.A &= data;
        registers.setStatus('N', helperByteOps::isBit7Set(registers.A));
        registers.setStatus('Z', registers.A == 0);
    }
    /* void ASL
    Performs a shift left on either the accumulator or some region in memory.
    
    Flags Affected:
     - C: set to value of old bit 7
     - Z: set if A = 0
     - N: set to value of new bit 7
    */
    void ASL(Registers& registers, uint8_t data) {
        registers.setStatus('C', helperByteOps::isBit7Set(registers.A));
        registers.A = registers.A << 1;
        registers.setStatus('N', helperByteOps::isBit7Set(registers.A));
        registers.setStatus('Z', registers.A == 0);
    }
    void ASL(Registers& registers, DataBus& dataBus, uint16_t address) {
        registers.setStatus('C', helperByteOps::isBit7Set(dataBus.read(address)));
        dataBus.write(address, dataBus.read(address) << 1);
        registers.setStatus('N', helperByteOps::isBit7Set(dataBus.read(address)));
        registers.setStatus('Z', dataBus.read(address) == 0);
    }
    /* void BCC
    Performs a branch if the carry flag is 0.

    Flags Affected:
        None

    */
    void BCC(Registers& registers, uint8_t data, bool& branched) {
        int8_t offset = data;  // Treat the data as signed.
        if (!registers.getStatus('C')) {
            registers.PC += offset;
            branched = true;
        } else {
            branched = false;
        }
        // if (offset >= 0 || registers.getStatus('C')) {
        registers.PC += 2;  // In that case, iterate the program counter manually since the 
        //}
    }
    /* void BCS
    Performs a branch if the carry flag is 1.

    Flags Affected:
        None
    */
    void BCS(Registers& registers, uint8_t data, bool& branched) {
        int8_t offset = data;  // Treat the data as signed.
        if (registers.getStatus('C')) {
            registers.PC += data;
            branched = true;
        } else {
            branched = false;
        }
        // if (offset >= 0 || !registers.getStatus('C')) {
        registers.PC += 2;  // In that case, iterate the program counter manually since the 
        //}
    }
    /* void BEQ
    Performs a branch if the zero flag is 1.
    The zero flag should have been set by another opcode [TODO: name it] before.

    Flags Affected:
        None

    */
    void BEQ(Registers& registers, uint8_t data, bool& branched) {
        int8_t offset = data;  // Treat the data as signed.
        if (registers.getStatus('Z')) {
            registers.PC += offset;
            branched = true;
        } else {
            branched = false;
        }
        registers.PC += 2;
    }
    /* void BIT
    Test if some bits are set in a memory location by doing a bitwise AND w/ the accumulator
    which acts as the mask.

    Example:
    0b10110100 = A
    0b11010011 = M
    0b10010000 = A & M

    After the operation, N and V would be set to 1, and Z is set to 0.

    Example 2:
    0b10100100 = A
    0b01010011 = M
    0b00000000 = A & M

    After the operation, N and V becomes 0 and 1 respectively, and Z is set to 1.

    Flags Affected:
        - N: Set to bit 7 of the memory value.
        - V: Set to bit 6 of the memory value.
        - Z: Set if the result is 0, unset otherwise.
    */
    void BIT(Registers& registers, uint8_t data) {
        uint8_t memValue = data;  // Treat the data as signed.
        registers.setStatus('N', memValue & 0b10000000);
        registers.setStatus('V', memValue & 0b01000000);
        registers.setStatus('Z', (memValue & registers.A) == 0);
    }
    /* void BMI
    Branches if the negative flag is set.

    Flags Affected:
        None
    */
    void BMI(Registers& registers, uint8_t data, bool& branched) {
        int8_t offset = data;  // Treat the data as signed.
        if (registers.getStatus('N')) {
            registers.PC += offset;
            branched = true;
        } else {
            branched = false;
        }
        // if (offset >= 0 || !registers.getStatus('N')) {
        registers.PC += 2;  // In that case, iterate the program counter manually since the 
        //}
    }
    /* void BNE
    Branches if the zero flag is 0.

    Flags Affected:
        None
    */
    void BNE(Registers& registers, uint8_t data, bool& branched) {
        int8_t offset = data;  // Treat the data as signed.
        if (!registers.getStatus('Z')) {
            registers.PC += offset;
            branched = true;
        } else {
            branched = false;
        }
        // if (offset >= 0 || registers.getStatus('Z')) {
        registers.PC += 2;  // In that case, iterate the program counter manually since the 
        //}
    }
    /* void BPL
    Branches if the negative flag is 0.

    Flags Affected:
        None
    */
    void BPL(Registers& registers, uint8_t data, bool& branched) {
        int8_t offset = data;  // Treat the data as signed.
        if (!registers.getStatus('N')) {
            registers.PC += offset;
            branched = true;
        } else {
            branched = false;
        }
        // if (offset >= 0 || registers.getStatus('N')) {
        registers.PC += 2;  // In that case, iterate the program counter manually since the 
        //}
    }
    /* void BRK
    Interrupt request as an instruction. Note that an interrupt can be requested outside the CPU and it will have the same effect as this instruction.

    Push the program counter then the status flags onto the stack.
    Then load the address stored in the IRQ interrupt vector (located at 0xfffe and 0xffff).
    Set the break flag in the status to 1.

    See NESDev for more details.

    Flags Affected:
        - B: set to 1.
    */
    void BRK(Registers& registers, DataBus& dataBus, uint16_t address) {
        // First, push the PC + 2 and Status Flags in the stack.
        // NOTE: I don't know if I need to push the current PC, +1, or +2 onto the stack.
        // NOTE: This code is duplicated in _6502_CPU; maybe I can fix that?
        dataBus.write(STACK_END_ADDR + registers.SP, registers.PC + 1);
        dataBus.write(STACK_END_ADDR + registers.SP - 1, (registers.PC + 1) >> 8);
        dataBus.write(STACK_END_ADDR + registers.SP - 2, registers.S);

        // Then, get the IRQ Interrupt Vector
        // TODO: Get rid of magic numbers.
        // Magic numbers: 0xfffe and 0xffff are the addresses where the IRQ vector is located.
        // lb = lower byte; ub = upper byte.
        uint8_t lb = dataBus.read(0xfffe);
        uint8_t ub = dataBus.read(0xffff);
        uint16_t irqVector = lb + (static_cast<uint16_t>(ub) << 8);

        // Finally, update the status flags.
        registers.PC = irqVector;
        registers.SP -= 3;
    }
    /* void BVC
    Branches if the overflow flag is 0.

    Flags Affected:
        None
    */
    void BVC(Registers& registers, uint8_t data, bool& branched) {
        int8_t offset = data;  // Treat the data as signed.
        if (!registers.getStatus('V')) {
            registers.PC += offset;
            branched = true;
        } else {
            branched = false;
        }
        // if (offset >= 0 || registers.getStatus('V')) {  // Iterate the PC by two if we are going forward OR if we are not branching at all.
        registers.PC += 2;
        //}
    }
    /* void BVS
    Branches if the overflow flag is 0.

    Flags Affected:
        None
    */
    void BVS(Registers& registers, uint8_t data, bool& branched) {
        int8_t offset = data;  // Treat the data as signed.
        if (registers.getStatus('V')) {
            registers.PC += offset;
            branched = true;
        }  else {
            branched = false;
        }
        // if (offset >= 0 || !registers.getStatus('V')) {
            registers.PC += 2;  // In that case, iterate the program counter manually since the 
        //}
    }
    /* void CLC
    Sets carry flag to 0.

    Flags Affected:
        - C: set to 0.
    */
    void CLC(Registers& registers, uint8_t data) {
        registers.setStatus('C', false);
    }
    /* void CLD
    Sets decimal flag to 0.
    Flags Affected:
        - D: set to 0.
    */
    void CLD(Registers& registers, uint8_t data) {
        registers.setStatus('D', 0);
    }
    /* void CLI
    Sets interrupt disable flag to 0.

    Flags Affected:
        - I: set to 0.
    */
    void CLI(Registers& registers, uint8_t data) {
        registers.setStatus('I', false);
    }
    /* void CLV
    Sets overflow flag to 0.

    Flags Affected:
        - V: set to 0.
    */
    void CLV(Registers& registers, uint8_t data) {
        registers.setStatus('V', false);
    }
    /* void CMP
    Compares the accumulator w/ the value in memory. 
    Sets some flags based on the result A-M (which is not stored)
    which is to be used later in the branch instructions.

    Flags Affected:
        - C: set to 1 if A >= M  // TODO: Find if it gets cleared if otherwise.
        - Z: set to 1 if A = M
        - N: set if A < M; This is checked by subtracting A by M, then seeing if the 7th bit is set.
    */
    void CMP(Registers& registers, uint8_t data) {
        registers.setStatus('C', registers.A >= data);
        registers.setStatus('Z', registers.A == data);
        registers.setStatus('N', helperByteOps::isLessThan(registers.A, data));
    }
    /* void CPX
    Compares the X register w/ the value in memory. 
    Sets some flags based on the result A-M (which is not stored)
    which is to be used later in the branch instructions.

    Flags Affected:
        - C: set to 1 if X >= M  // TODO: Find if it gets cleared if otherwise.
        - Z: set to 1 if X = M
        - N: set if X < M
    */
    void CPX(Registers& registers, uint8_t data) {
        registers.setStatus('C', registers.X >= data);
        registers.setStatus('Z', registers.X == data);
        registers.setStatus('N', helperByteOps::isLessThan(registers.X, data));
    } 
    /* void CPY
    Compares the Y register w/ the value in memory.
    Sets some flags based on the result A-M (which is not stored)
    which is to be used later in the branch instructions.

    Flags Affected:
        - C: set to 1 if Y >= M  // TODO: Find if it gets cleared if otherwise.
        - Z: set to 1 if Y = M
        - N: set if Y < M
    */
    void CPY(Registers& registers, uint8_t data) {
        registers.setStatus('C', registers.Y >= data);
        registers.setStatus('Z', registers.Y == data);
        registers.setStatus('N', helperByteOps::isLessThan(registers.Y, data));
    }
    /* void DEC
    Decrements a given memory value by 1.

    Flags Affectected:
     - Z: If the result is 0.
     - N: If the 7th bit is set (indicating a negative value).
    */
    void DEC(Registers& registers, DataBus& dataBus, uint16_t address) {
        dataBus.write(address, dataBus.read(address) - 1);
        uint8_t newVal = dataBus.read(address);
        registers.setStatus('Z', newVal == 0);
        registers.setStatus('N', helperByteOps::isBit7Set(newVal));
    }
    /* void DEX
    Decrements the X register by 1.

    Flags Affectected:
     - Z: If the result is 0.
     - N: If the 7th bit is set (indicating a negative value).
    */
    void DEX(Registers& registers, uint8_t data) {
        --registers.X;
        registers.setStatus('Z', registers.X == 0);
        registers.setStatus('N', helperByteOps::isBit7Set(registers.X));
    }
    /* void DEY
    Decrements the Y register by 1.

    Flags Affectected:
     - Z: If the result is 0.
     - N: If the 7th bit is set (indicating a negative value).
    */
    void DEY(Registers& registers, uint8_t data) {
        --registers.Y;
        registers.setStatus('Z', registers.Y == 0);
        registers.setStatus('N', helperByteOps::isBit7Set(registers.Y));
    }
    /* void EOR
    Performs bitwise xor on the accumulator using the contents in memory.

    Flags Affectected:
     - Z: If A = 0.
     - N: If the 7th bit of A is set (indicating a negative value).
    */
    void EOR(Registers& registers, uint8_t data) {
        registers.A ^= data;
        registers.setStatus('Z', registers.A == 0);
        registers.setStatus('N', helperByteOps::isBit7Set(registers.A));
    }
    /* void INC
    Increments memory value by 1.

    Flags Affectected:
     - Z: If the result is 0.
     - N: If the 7th bit is set (indicating a negative value).
    */
    void INC(Registers& registers, DataBus& dataBus, uint16_t address) {
        uint8_t newVal = dataBus.write(address, dataBus.read(address) + 1);
        registers.setStatus('Z', newVal == 0);
        registers.setStatus('N', helperByteOps::isBit7Set(newVal));
    }
    /* void INX
    Increments X register by 1.

    Flags Affectected:
     - Z: If the result is 0.
     - N: If the 7th bit is set (indicating a negative value).
    */
    void INX(Registers& registers, uint8_t data) {
        ++registers.X;
        registers.setStatus('Z', registers.X == 0);
        registers.setStatus('N', helperByteOps::isBit7Set(registers.X));
    }
    /* void INY
    Increments the Y register by 1.

    Flags Affectected:
     - Z: If the result is 0.
     - N: If the 7th bit is set (indicating a negative value).
    */
    void INY(Registers& registers, uint8_t data) {
        ++registers.Y;
        registers.setStatus('Z', registers.Y == 0);
        registers.setStatus('N', helperByteOps::isBit7Set(registers.Y));
    }
    /* void JMP
    Sets program counter to address specified by the operand.

    Flags Affectected:
        None
    */
    void JMP(Registers& registers, DataBus& databus, uint16_t data) {
        registers.PC = data;
    }
    /* void JSR
    Pushes what would be the next value of the program counter MINUS 1 onto the stack.
    Sets program counter to address specified by the operand.

    e.g. If I am currently at hex value 0x0407, and the program counter would normally become 0x0409, the value
    0x0408 is pushed onto the stack 
    
    Sample memory slice:
    (00 00 08 04 00 00)
           ^^ ^^
    Values pushed onto stack (The 6502 is little endian, so the lower two bytes come first).

    Flags Affectected:
        None
    */
    void JSR(Registers& registers, DataBus& dataBus, uint16_t address) {
        const int instructionSize = 3;  // This only uses absolute addressing, and it will always be 3 bytes in length.
        registers.PC += 2;  // Add 3 to move to the next instruction, subtract 1 for this opcode = move PC by 2.
        uint8_t lowerByte, upperByte;
        lowerByte = registers.PC & 0xFF;
        upperByte = (registers.PC & 0xFF00) >> 8;
        dataBus.write(STACK_END_ADDR + registers.SP - 1, lowerByte);  // The stack pointer points to the vacant address right above the ones being used..
        dataBus.write(STACK_END_ADDR + registers.SP, upperByte);
        registers.PC = address;
        registers.SP -= 2;
    }
    /* void LDA
    Loads the data at a memory location into the accumulator.

    Flags Affected:
     - Z: set to 1 if A = 0.
     - N: set to 1 if bit 7 of A is set.
    */
    void LDA(Registers& registers, uint8_t data) {
        registers.A = data;
        registers.setStatus('Z', registers.A == 0);
        registers.setStatus('N', helperByteOps::isBit7Set(registers.A));
    }
    /* void LDX
    Loads the data at a memory location into the X registers.

    Flags Affected:
     - Z: set to 1 if X = 0.
     - N: set to 1 if bit 7 of X is set.
    */
    void LDX(Registers& registers, uint8_t data) {
        registers.X = data;
        auto a = registers.getStatus('V');
        auto c = registers.S & 0b00100000;
        registers.setStatus('Z', registers.X == 0);
        registers.setStatus('N', helperByteOps::isBit7Set(registers.X));
        auto b = registers.getStatus('V');
    }
    /* void LDY
    Loads the data at a memory location into the Y register.

    Flags Affected:
     - Z: set to 1 if Y = 0.
     - N: set to 1 if bit 7 of Y is set.
    */
    void LDY(Registers& registers, uint8_t data) {
        registers.Y = data;
        registers.setStatus('Z', registers.Y == 0);
        registers.setStatus('N', helperByteOps::isBit7Set(registers.Y));
    }
    /* void LSR
    Performs a shift right.

    Flags Affected:
     - C: set to value of old bit 7
     - Z: set if A = 0
     - N: set to value of new bit 7
    */
    void LSR(Registers& registers, uint8_t data) {
        registers.setStatus('C', helperByteOps::isBit0Set(registers.A));
        registers.A = registers.A >> 1;
        registers.setStatus('N', helperByteOps::isBit7Set(registers.A));  // Won't this always be false?
        registers.setStatus('Z', registers.A == 0);
    }
    void LSR(Registers& registers, DataBus& dataBus, uint16_t address) {
        registers.setStatus('C', helperByteOps::isBit0Set(dataBus.read(address)));
        dataBus.write(address, dataBus.read(address) >> 1);
        registers.setStatus('N', helperByteOps::isBit7Set(dataBus.read(address)));  // Won't this always be false?
        registers.setStatus('Z', dataBus.read(address) == 0);
    }
    /* void NOP
    Does not affect the processor; does nothing.

    Flags Affected:
        None
    */
    void NOP(Registers& registers, uint8_t data) {}
    void NOP(Registers& registers, DataBus& dataBus, uint16_t address) {}
    /* void ORA
    Performs logical OR on address and memory value.
    
    Flags Affected:
     - Z: set if A = 0.
     - N: set if bit 7 of A is set.
    */
    void ORA(Registers& registers, uint8_t data) {
        registers.A |= data;
        registers.setStatus('Z', registers.A == 0);
        registers.setStatus('N', helperByteOps::isBit7Set(registers.A));
    }
    /* void PHA
    Pushes a copy of the accumulator onto the stack.

    Flags Affected:
        None
    */
    void PHA(Registers& registers, DataBus& dataBus, uint16_t address) {
        if (!(0b00100000 & registers.S)) {  // Just validating the status
            ++registers.S;
            --registers.S;
        }
        dataBus.write(STACK_END_ADDR + registers.SP, registers.A);
        --registers.SP;
    }
    /* void PHP
    Pushes a copy of the status flags onto the stack. The B flag, while not set,
    will be treated as 1 when pushing onto the stack, so we OR it w/ the status byte.

    Flags Affected:
        None
    */
    void PHP(Registers& registers, DataBus& dataBus, uint16_t address) {
        if (!(0b00100000 & registers.S)) {  // Just validating the status
            ++registers.S;
            --registers.S;
        }
        dataBus.write(STACK_END_ADDR + registers.SP, registers.S | 0b00010000); 
        --registers.SP;
    }
    /* void PLA
    Pulls the accumulator from the stack.

    Flags Affected:
     - Z: set if A = 0.
     - N: set if bit 7 of A is 0.
    */
    void PLA(Registers& registers, DataBus& dataBus, uint16_t address) {
        registers.A = dataBus.read(STACK_END_ADDR + registers.SP + 1);
        registers.setStatus('Z', registers.A == 0);
        registers.setStatus('N', helperByteOps::isBit7Set(registers.A));
        ++registers.SP;
    }
    /* void PLA
    Pulls the status flags from the stack.

    Flags Affected:
        All flags are set to their respective values in the stack.
    */
    void PLP(Registers& registers, DataBus& dataBus, uint16_t address) {
        if (!(0b00100000 & registers.S)) {  // Just validating the status
            ++registers.S;
            --registers.S;
        }
        registers.S = dataBus.read(STACK_END_ADDR + registers.SP + 1);
        registers.S &= 0b11101111;  // Make sure the B flag is 0; regardless of its actual value in the stack.
        registers.S |= 0b00100000;  // Make sure the 1 flag is 0; regardless of its actual value in the stack
        ++registers.SP;
    }
    /* void ROL
    Moves all bits in the accumulator or memory location to the left by one bit.
    Bit 0 takes on the value of the old carry flag value, then the carry flag becomes the value of the old bit 7.

    Flags Affected:
     - C: set to the old value of bit 7.
     - Z: set if the new value = 0.
     - N: set if bit 7 of the new value is set.
    */
    void ROL(Registers& registers, uint8_t data) {
        bool oldBit7 = helperByteOps::isBit7Set(registers.A);
        registers.A <<= 1;
        registers.A += registers.getStatus('C');
        registers.setStatus('C', oldBit7);
        registers.setStatus('Z', registers.A == 0);
        registers.setStatus('N', helperByteOps::isBit7Set(registers.A));
    }
    void ROL(Registers& registers, DataBus& dataBus, uint16_t address) {
        uint8_t tempVal = dataBus.read(address);
        bool oldBit7 = helperByteOps::isBit7Set(dataBus.read(address));
        tempVal <<= 1;
        tempVal += registers.getStatus('C');
        registers.setStatus('C', oldBit7);
        registers.setStatus('Z', tempVal == 0);
        registers.setStatus('N', helperByteOps::isBit7Set(tempVal));
        dataBus.write(address, tempVal);
    }
    /* void ROR
    Moves all bits in the accumulator or memory location to the right by one bit.
    Bit 7 takes on the value of the old carry flag value, then the carry flag becomes the value of the old bit 0.

    Flags Affected:
     - C: set to the old value of bit 0.
     - Z: set if the new value = 0.
     - N: set if bit 7 of the new value is set.
    */
    void ROR(Registers& registers, uint8_t data) {
        bool oldBit0 = helperByteOps::isBit0Set(registers.A);
        registers.A >>= 1;
        registers.A += registers.getStatus('C') << 7;
        registers.setStatus('C', oldBit0);
        registers.setStatus('Z', registers.A == 0);
        registers.setStatus('N', helperByteOps::isBit7Set(registers.A));
    }
    void ROR(Registers& registers, DataBus& dataBus, uint16_t address) {
        uint8_t tempVal = dataBus.read(address);
        bool oldBit0 = helperByteOps::isBit0Set(dataBus.read(address));
        tempVal >>= 1;
        tempVal += registers.getStatus('C') << 7;
        registers.setStatus('C', oldBit0);
        registers.setStatus('Z', tempVal == 0);
        registers.setStatus('N', helperByteOps::isBit7Set(tempVal));
        dataBus.write(address, tempVal);
    }
    /* void RTI
    Returns from an interrupt; pulls the program counter, then the status flags.

    Flags Affected:
        All flags are set from the stack.
    
    Bug List:
     - The constant 1 in the Status is not 1 after this is executed.
    Instructions executed prior:
        BNE RELTV | Operands: 0x23, ____ | Old values of A: 0x55, X: 0x99, Y: 0x88, SP: 0x80, PC: 0xcec0 | Flags 0x67 C:1, Z: 1, I: 1, D: 0, V: 1, N: 0
        Instruction executed (8):
        LDA IMMED | Operands: 0xce, ____ | Old values of A: 0x55, X: 0x99, Y: 0x88, SP: 0x80, PC: 0xcec2 | Flags 0x67 C:1, Z: 1, I: 1, D: 0, V: 1, N: 0
        Instruction executed (9):
        PHA IMPLD | Operands: ____, ____ | Old values of A: 0xce, X: 0x99, Y: 0x88, SP: 0x80, PC: 0xcec4 | Flags 0xe5 C:1, Z: 0, I: 1, D: 0, V: 1, N: 1
        Instruction executed (10):
        LDA IMMED | Operands: 0xce, ____ | Old values of A: 0xce, X: 0x99, Y: 0x88, SP: 0x7f, PC: 0xcec5 | Flags 0xe5 C:1, Z: 0, I: 1, D: 0, V: 1, N: 1
        Instruction executed (11):
        PHA IMPLD | Operands: ____, ____ | Old values of A: 0xce, X: 0x99, Y: 0x88, SP: 0x7f, PC: 0xcec7 | Flags 0xe5 C:1, Z: 0, I: 1, D: 0, V: 1, N: 1
        Instruction executed (12):
        LDA IMMED | Operands: 0x87, ____ | Old values of A: 0xce, X: 0x99, Y: 0x88, SP: 0x7e, PC: 0xcec8 | Flags 0xe5 C:1, Z: 0, I: 1, D: 0, V: 1, N: 1
        Instruction executed (13):
        PHA IMPLD | Operands: ____, ____ | Old values of A: 0x87, X: 0x99, Y: 0x88, SP: 0x7e, PC: 0xceca | Flags 0xe5 C:1, Z: 0, I: 1, D: 0, V: 1, N: 1
        Instruction executed (14):
        LDA IMMED | Operands: 0x55, ____ | Old values of A: 0x87, X: 0x99, Y: 0x88, SP: 0x7d, PC: 0xcecb | Flags 0xe5 C:1, Z: 0, I: 1, D: 0, V: 1, N: 1
    */
    void RTI(Registers& registers, DataBus& dataBus, uint16_t address) {
        registers.S = dataBus.read(registers.SP + STACK_END_ADDR + 1);
        ++registers.SP;
        registers.PC = dataBus.read(registers.SP + STACK_END_ADDR + 1);
        ++registers.SP;
        registers.PC += static_cast<uint16_t>(dataBus.read(registers.SP + STACK_END_ADDR + 1)) << 8;
        ++registers.SP;
        --registers.PC;  // Account for the fact the CPU will advance the PC by 1 for this instruction.
        // If you don't decrement the PC, it will be erroneously 1 byte ahead
    }
    /* void RTS
    Returns from the subroutine by going to the address stored in the stack.

    Flags Affected:
        None
    */
    void RTS(Registers& registers, DataBus& dataBus, uint16_t address) {
        uint8_t lowerByte = dataBus.read(registers.SP + STACK_END_ADDR + 1);
        uint8_t upperByte = dataBus.read(registers.SP + STACK_END_ADDR + 2);
        // Remember that we stored the address we meant to go to next MINUS 1? So we must add it now.
        uint16_t returnAddress = lowerByte + (upperByte << 8) + 1;
        registers.PC = returnAddress;
        registers.SP += 2;
    }
    /* void SBC
    Subtracts the content of the accumulator with the value in the memory and NOT of the carry flag.
    i.e. A = A - M - (1 - C)

    Flags Affected:
     - C: 0 if overflow in bit 7.
    BUGS:
     Expected log:
        DB52  A9 81     LDA #$81                        A:7F X:05 Y:00 P:65 SP:FB PPU: 83,194 CYC:9499
        DB54  F1 33     SBC ($33),Y = 0400 @ 0400 = 7F  A:81 X:05 Y:00 P:E5 SP:FB PPU: 83,200 CYC:9501
        DB56  50 06     BVC $DB5E                       A:02 X:05 Y:00 P:65 SP:FB PPU: 83,215 CYC:9506
     Actual:
        (1209): LDA IMMED | Operands: 0x81, ____ | Old values of A: 0x7f, X: 0x05, Y: 0x00, SP: 0xfb, PC: 0xdb52 | Flags 0x65 C:1, Z: 0, I: 1, D: 0, V: 1, N: 0 | (3170)
        (1210): SBC INDXY | Operands: 0x33, ____ | Old values of A: 0x81, X: 0x05, Y: 0x00, SP: 0xfb, PC: 0xdb54 | Flags 0xe5 C:1, Z: 0, I: 1, D: 0, V: 1, N: 1 | (3171)
        (1211): BVC RELTV | Operands: 0x06, ____ | Old values of A: 0x7b, X: 0x05, Y: 0x00, SP: 0xfb, PC: 0xdb56 | Flags 0x64 C:0, Z: 0, I: 1, D: 0, V: 1, N: 0 | (3172)

     */
    void SBC(Registers& registers, uint8_t data) {
        ops::ADC(registers, ~data);
    }
    /* void SEC
    Sets the carry flag to 1.

    Flags Affected:
     - C: set to 1.
    */
    void SEC(Registers& registers, uint8_t data) {
        registers.setStatus('C', 1);
    }
    /* void SED
    Sets decimal flag to 1.

    Flags Affected:
     - D: set to 1.
    */
    void SED(Registers& registers, uint8_t data) {
        registers.setStatus('D', 1);
    }
    /* void SEI
    Sets interrupt disable flag to 1.

    Flags Affected:
     - I: set to 1.
    */
    void SEI(Registers& registers, uint8_t data) {
        registers.setStatus('I', 1);
    }
    /* void STA
    Stores the accumulator in a memory location.

    Flags Affected:
        None
    */
    void STA(Registers& registers, DataBus& dataBus, uint16_t address) {
        dataBus.write(address, registers.A);
    }
    /* void STX
    Stores the X register in a memory location.

    Flags Affected:
        None
    */
    void STX(Registers& registers, DataBus& dataBus, uint16_t address) {
        dataBus.write(address, registers.X);
    }
    /* void STY
    Stores the Y register in a memory location.

    Flags Affected:
        None
    */
    void STY(Registers& registers, DataBus& dataBus, uint16_t address) {
        dataBus.write(address, registers.Y);
    }
    /* void TAX
    Copies the accumulator into the X register.

    Flags Affected:
     - Z: set if X = 0.
     - N: set if bit 7 of X is set.
    */
    void TAX(Registers& registers, uint8_t data) {
        registers.X = registers.A;
        registers.setStatus('Z', registers.X == 0);
        registers.setStatus('N', helperByteOps::isBit7Set(registers.X));
    }
    /* void TAY
    Copies the accumulator into the Y register.

    Flags Affected:
     - Z: set if Y = 0.
     - N: set if bit 7 of Y is set.
    */
    void TAY(Registers& registers, uint8_t data) {
        registers.Y = registers.A;
        registers.setStatus('Z', registers.Y == 0);
        registers.setStatus('N', helperByteOps::isBit7Set(registers.Y));
    }
    /* void TSR
    Copies the stack pointer into the X register.

    Flags Affected:
     - Z: set if X = 0.
     - N: set if bit 7 of X is set.
    */
    void TSX(Registers& registers, uint8_t data) {
        registers.X = registers.SP;
        registers.setStatus('Z', registers.X == 0);
        registers.setStatus('N', helperByteOps::isBit7Set(registers.X));
    }
    /* void TXA
    Copies the X register into the accumulator.

    Flags Affected:
     - Z: set if A = 0.
     - N: set if bit 7 of A is set.
    */
    void TXA(Registers& registers, uint8_t data) {
        registers.A = registers.X;
        registers.setStatus('Z', registers.A == 0);
        registers.setStatus('N', helperByteOps::isBit7Set(registers.A));
    }
    /* void TXS
    Copies the X register into the accumulator.

    Flags Affected:
        None
    */
    void TXS(Registers& registers, uint8_t data) {
        registers.SP = registers.X;
    }
    /* void TYA
    Copies the Y register into the accumulator.

    Flags Affected:
     - Z: set if A = 0.
     - N: set if bit 7 of A is set.
    */
    void TYA(Registers& registers, uint8_t data) {
        registers.A = registers.Y;
        registers.setStatus('Z', registers.A == 0);
        registers.setStatus('N', helperByteOps::isBit7Set(registers.A));
    }
}
