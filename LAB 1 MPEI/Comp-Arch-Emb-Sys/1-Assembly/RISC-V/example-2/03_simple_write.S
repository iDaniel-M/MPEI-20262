// Directives to make interesting windows visible
#pragma qtrvsim show registers
#pragma qtrvsim show memory
#pragma qtrvsim show peripherals

.equ SPILED_REG_BASE,       0xffffc100 // base of SPILED port region

.equ SPILED_REG_LED_LINE,   0xffffc104 // 32 bit word mapped as output
.equ SPILED_REG_LED_LINE_o,     0x0004 // Offset of the LED_LINE
.equ SPILED_REG_LED_RGB1,   0xffffc110 // RGB LED 1 color components
.equ SPILED_REG_LED_RGB1_o,     0x0010 // Offset of LED_RGB1
.equ SPILED_REG_LED_RGB2,   0xffffc114 // RGB LED 2 color components
.equ SPILED_REG_LED_RGB2_o,     0x0014 // Offset of LED_RGB2
.equ SPILED_REG_KNOBS_8BIT, 0xffffc124 // Three 8 bit knob values
.equ SPILED_REG_KNOBS_8BIT_o,   0x0024 // Offset of KNOBS_8BIT

// Start address after reset
.org 0x00000200

.text
_start:
    li a0, SPILED_REG_BASE           // load base address of LED port
    li a1, 0xaaaa                    // Load color
    sw a1, SPILED_REG_LED_RGB1_o(a0) // Write color to memory mapped IO

    ebreak                           // Stop the program

// if whole source compile is OK the switch to core tab
#pragma qtrvsim tab core