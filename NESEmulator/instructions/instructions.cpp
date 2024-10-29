#include "instructions.h"
#include "../6502Chip/CPU.h"

namespace addressingOperations {
    uint8_t immediate(DataBus& databus, Registers& registers) {
        uint8_t data = databus.read(registers.programCounter + 1);
        return data;
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
        registers.accumulator += data;
        if (0b10000000 & registers.accumulator) {
            registers.setStatus('N', true);
        }
        else {
            registers.setStatus('N', false);
        }

        if (registers.accumulator == 0) {
            registers.setStatus('Z', true);
        }
        else {
            registers.setStatus('Z', false);
        }
    }
}