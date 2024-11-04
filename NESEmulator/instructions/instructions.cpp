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

namespace addrModes {
    uint16_t immediate(DataBus& databus, Registers& registers, bool fetchTwoBytes) {
        // The value after the opcode is treated as data and not an address.
        uint16_t data = databus.read(registers.PC + 1);
        if (fetchTwoBytes) {
            data += static_cast<uint16_t>(databus.read(registers.PC + 2)) << 8;
        }
        return data;
    }

    uint16_t implicit(DataBus& databus, Registers& registers, bool fetchTwoBytes) {
        // Implicit instructions do not use any value from RAM. This 0 will go unused.
        return 0;  
    }

    uint16_t accumulator(DataBus& databus, Registers& registers, bool fetchTwoBytes) {
        // Instructions with this addressing mode operate on the accumulator; not on RAM.
        return registers.A;
    }

    uint16_t zeropage(DataBus& databus, Registers& registers, bool fetchTwoBytes) {
        // Indexes 0x00LL; zeropage takes fewer cycles than other addressing modes.
        uint16_t address = static_cast<uint16_t>(databus.read(registers.PC + 1));
        return databus.read(address);;
    }
    uint16_t zeropageX(DataBus& databus, Registers& registers, bool fetchTwoBytes) {
        // Indexes 0x00LL + X; zeropage takes fewer cycles than other addressing modes.
        uint16_t address = static_cast<uint16_t>(databus.read(registers.PC + 1));
        return databus.read(address + registers.X);
    }
    uint16_t zeropageY(DataBus& databus, Registers& registers, bool fetchTwoBytes) {
        // Indexes 0x00LL + Y; zeropage takes fewer cycles than other addressing modes.
        uint16_t address = static_cast<uint16_t>(databus.read(registers.PC + 1));
        return databus.read(address + registers.Y);
    }
    uint16_t relative(DataBus& databus, Registers& registers, bool fetchTwoBytes) {
        // The offset is a signed byte
        int8_t offset = databus.read(registers.PC + 1);
        uint16_t data = databus.read(registers.PC + offset);
        if (fetchTwoBytes) {
            data += static_cast<uint16_t>(databus.read(registers.PC + offset + 1)) << 8;
        }
        return data;
    }
    uint16_t absolute(DataBus& databus, Registers& registers, bool fetchTwoBytes) {
        // The NES is a little-endian machine, meaning the FIRST byte read takes up the LOWER 8 bits.
        // So if in memory we had these values: 8f 30
        // We would return a memory address of 0x308f
        // lb = lower byte; ub = upper byte; addr = address
        uint16_t lbOfAddr = static_cast<uint16_t>(databus.read(registers.PC + 1));
        uint16_t ubOfAddr = static_cast<uint16_t>(databus.read(registers.PC + 2)) << 8;
        uint16_t data = databus.read(lbOfAddr + ubOfAddr);
        if (fetchTwoBytes) {
            data += static_cast<uint16_t>(databus.read(lbOfAddr + ubOfAddr + 1)) << 8;
        }
        return data;
    }
    uint16_t absoluteX(DataBus& databus, Registers& registers, bool fetchTwoBytes) {
        // The NES is a little-endian machine, meaning the FIRST byte read takes up the LOWER 8 bits.
        // So if in memory we had these values: 8f 30
        // We would return a memory address of 0x308f + X 
        // lb = lower byte; ub = upper byte; addr = address
        uint16_t lbOfAddr = static_cast<uint16_t>(databus.read(registers.PC + 1));
        uint16_t ubOfAddr = static_cast<uint16_t>(databus.read(registers.PC + 2)) << 8;
        uint16_t data = databus.read(lbOfAddr + ubOfAddr + registers.X);
        if (fetchTwoBytes) {
            data += static_cast<uint16_t>(databus.read(lbOfAddr + ubOfAddr + registers.X + 1)) << 8;
        }
        return data;
    }
    uint16_t absoluteY(DataBus& databus, Registers& registers, bool fetchTwoBytes) {
        // The NES is a little-endian machine, meaning the FIRST byte read takes up the LOWER 8 bits.
        // So if in memory we had these values: 8f 30
        // We would return a memory address of 0x308f + Y
        // lb = lower byte; ub = upper byte; addr = address
        uint16_t lbOfAddr = static_cast<uint16_t>(databus.read(registers.PC + 1));
        uint16_t ubOfAddr = static_cast<uint16_t>(databus.read(registers.PC + 2)) << 8;
        uint16_t data = databus.read(lbOfAddr + ubOfAddr + registers.Y);
        if (fetchTwoBytes) {
            data += static_cast<uint16_t>(databus.read(lbOfAddr + ubOfAddr + registers.Y + 1)) << 8;
        }
        return data;
    }

    // Works similar to pointers to addresses. 
    // It first goes to the given memory address, looks at the byte and the byte 
    // of the next address, uses those two bytes to make a new address 
    // which it gets the value of.
    uint16_t indirect(DataBus& databus, Registers& registers, bool fetchTwoBytes) {
        // lb = lower byte; ub = upper byte; addr = address

        // First, we get the values of the next 2 bytes (the address contained in the pointer)
        uint16_t lbOfPtrAddr = static_cast<uint16_t>(databus.read(registers.PC + 1));
        uint16_t ubOfPtrAddr = static_cast<uint16_t>(databus.read(registers.PC + 2)) << 8;
        // We then find the address of the data located at the address given by the pointer.
        uint16_t lbOfAddr = databus.read(lbOfPtrAddr + ubOfPtrAddr);
        uint16_t ubOfAddr = databus.read(lbOfPtrAddr + ubOfPtrAddr + 1) << 8;

        // Finally we get the data.
        uint16_t data = databus.read(lbOfAddr + ubOfAddr);
        if (fetchTwoBytes) {
            data += static_cast<uint16_t>(databus.read(lbOfAddr + ubOfAddr + 1)) << 8;
        }
        return data;

    }
    uint16_t indirectX(DataBus& databus, Registers& registers, bool fetchTwoBytes) {
        // lb = lower byte; ub = upper byte; addr = address
        // This addressing mode is zeropage.

        // First, we get the values of the pointer.
        uint16_t ptrAddr = static_cast<uint16_t>(databus.read(registers.PC + 1));
        // We then find the address located at the address given by the pointer + the X register.
        uint16_t addr = databus.read(ptrAddr + registers.X);

        // Then we get the data at this address.
        uint16_t data = databus.read(addr);
        if (fetchTwoBytes) {
            data += static_cast<uint16_t>(databus.read(addr + 1)) << 8;
        }
        return data;
    } 
    uint16_t indirectY(DataBus& databus, Registers& registers, bool fetchTwoBytes) {
        // lb = lower byte; ub = upper byte; addr = address
        // This addressing mode is zeropage.

        // First, we get the values of the pointer.
        uint16_t ptrAddr = static_cast<uint16_t>(databus.read(registers.PC + 1));
        // We then find the address located at the address given by the pointer.
        uint16_t addr = databus.read(ptrAddr);

        // Finally
        uint16_t data = databus.read(addr + registers.Y);
        if (fetchTwoBytes) {
            data += static_cast<uint16_t>(databus.read(addr + registers.Y + 1)) << 8;
        }
        return data;
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
    void ADC(Registers& registers, uint16_t data) {
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
    void AND(Registers& registers, uint16_t data) {
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
    void ASL(Registers& registers, uint16_t data) {
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
    void BCC(Registers& registers, uint16_t data) {
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
    void BCS(Registers& registers, uint16_t data) {
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
    void BEQ(Registers& registers, uint16_t data) {
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
    void BIT(Registers& registers, uint16_t data) {
        int8_t offset = data;  // Treat the data as signed.
        if (registers.getStatus('C')) {
            registers.PC += offset;
        }
    }
    /* void BMI
    Branches if the negative flag is set.

    Flags Affected:
        None
    */
    void BMI(Registers& registers, uint16_t data) {
        int8_t offset = data;  // Treat the data as signed.
        if (registers.getStatus('N')) {
            registers.PC += offset;
        }
    }
    /* void BNE
    Branches if the zero flag is 0.

    Flags Affected:
        None
    */
    void BNE(Registers& registers, uint16_t data) {
        int8_t offset = data;  // Treat the data as signed.
        if (!registers.getStatus('Z')) {
            registers.PC += offset;
        }
    }
    /* void BPL
    Branches if the negative flag is 0.

    Flags Affected:
        None
    */
    void BPL(Registers& registers, uint16_t data) {
        int8_t offset = data;  // Treat the data as signed.
        if (!registers.getStatus('N')) {
            registers.PC += offset;
        }
    }
    /* void BRK
    Branches if the negative flag is 0.

    TODO: fully implement.

    Flags Affected:
        - B: set to 1.
    */
    void BRK(Registers& registers, DataBus& dataBus, uint16_t data) {
        dataBus.write(registers.SP, registers.PC);
        dataBus.write(registers.SP + 1, registers.PC);
        ++++registers.SP;
    }
    /* void BVC
    Branches if the overflow flag is 0.

    Flags Affected:
        None
    */
    void BVC(Registers& registers, uint16_t data) {
        int8_t offset = data;  // Treat the data as signed.
        if (!registers.getStatus('V')) {
            registers.PC += offset;
        }
    }
    /* void BVS
    Branches if the overflow flag is 0.

    Flags Affected:
        None
    */
    void BVS(Registers& registers, uint16_t data) {
        int8_t offset = data;  // Treat the data as signed.
        if (registers.getStatus('V')) {
            registers.PC += offset;
        }
    }
    /* void CLC
    Sets carry flag to 0.

    Flags Affected:
        - C: set to 0.
    */
    void CLC(Registers& registers, uint16_t data) {
        registers.setStatus('C', false);
    }
    /* void CLI
    Sets interrupt disable flag to 0.

    Flags Affected:
        - I: set to 0.
    */
    void CLI(Registers& registers, uint16_t data) {
        registers.setStatus('I', false);
    }
    /* void CLV
    Sets overflow flag to 0.

    Flags Affected:
        - V: set to 0.
    */
    void CLV(Registers& registers, uint16_t data) {
        registers.setStatus('V', false);
    }
    /* void CMP
    Compares the accumulator w/ the value in memory. 
    Sets some flags based on the result A-M (which is not stored)
    which is to be used later in the branch instructions.

    Flags Affected:
        - C: set to 1 if A >= M  // TODO: Find if it gets cleared if otherwise.
        - Z: set to 1 if A = M
        - N: set if A < M
    */
    void CMP(Registers& registers, uint16_t data) {
        registers.setStatus('C', registers.A >= data);
        registers.setStatus('Z', registers.A == data);
        registers.setStatus('N', registers.A < data);
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
    void CPX(Registers& registers, uint16_t data) {

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
    void CPY(Registers& registers, uint16_t data)
    {
    }
    void DEC(Registers& registers, DataBus& databus, uint16_t data)
    {
    }
    void DEX(Registers& registers, uint16_t data)
    {
    }
    void DEY(Registers& registers, uint16_t data)
    {
    }
    void INC(Registers& registers, uint16_t data)
    {
    }
    /* void LDA
    Loads the data at a memory location into the accumulator

    Flags Affected:
     - Z: set to 1 if A = 0.
     - N: set to 1 if bit 7 is set.
    */
    void LDA(Registers& registers, uint16_t data) {
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
    void JMP(Registers& registers, uint16_t data)
    {
    }
    void JSR(Registers& registers, DataBus& databus, uint16_t data)
    {
    }
    void RTS(Registers& registers, DataBus& databus, uint16_t data)
    {
    }
    void ROR(Registers& registers, uint16_t data)
    {
    }
    void ROL(Registers& registers, uint16_t data)
    {
    }
    void SBC(Registers& registers, uint16_t data)
    {
    }
    void SEC(Registers& registers, uint16_t data)
    {
    }
    void SEI(Registers& registers, uint16_t data)
    {
    }
    void STX(Registers& registers, DataBus& databus, uint16_t data)
    {
    }
    void STY(Registers& registers, DataBus& databus, uint16_t data)
    {
    }
    void TAY(Registers& registers, uint16_t data)
    {
    }
    void TAX(Registers& registers, uint16_t data)
    {
    }
    void TSX(Registers& registers, uint16_t data)
    {
    }
    void TXA(Registers& registers, uint16_t data)
    {
    }
    void TXS(Registers& registers, uint16_t data)
    {
    }
    void TYA(Registers& registers, uint16_t data)
    {
    }
}
