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
   
   addi x3, x0, 0x100                   // initialize loop counter

l1:
    li x1, SPILED_REG_BASE              // load base address of LED port
    lw x2, SPILED_REG_KNOBS_8BIT_o(x1)  // Read colors from knobs


    sw x2, SPILED_REG_LED_RGB1_o(x1)    // Write color to led 1 

    addi x3, x3, -1                     // decrement counter
    bgtz x3, l1                         // loop back



    ebreak                              // Stop the program

// if whole source compile is OK the switch to core tab
#pragma qtrvsim tab core

