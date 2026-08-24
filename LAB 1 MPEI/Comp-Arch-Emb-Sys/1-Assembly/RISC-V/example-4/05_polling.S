// Directives to make interesting windows visible
#pragma qtrvsim show terminal
#pragma qtrvsim show registers
#pragma qtrvsim show memory

.equ SERIAL_PORT_BASE,      0xffffc000 // base address of serial port region

.equ SERP_TX_ST_REG,        0xffffc008 // Transmitter status register
.equ SERP_TX_ST_REG_o,          0x0008 // Offset of TX_ST_REG
.equ SERP_TX_ST_REG_READY_m,       0x1 // Transmitter can accept next byte
.equ SERP_TX_ST_REG_IE_m,          0x2 // Enable Tx ready interrupt

.equ SERP_TX_DATA_REG,      0xffffc00c // Write word to send 8 LSB bits to terminal
.equ SERP_TX_DATA_REG_o,        0x000c // Offset of TX_DATA_REG

_start:
loop:
    li   x1, SERIAL_PORT_BASE           // load base address of serial port
    addi x2, zero, text_1               // load address of text

next_char:
    lb   x3, 0(x2)                      // load one byte after another
    beq  x3, zero, end_char             // is this the terminal zero byte
    addi x2, x2, 1                      // move pointer to next text byte
tx_busy:
    lw   x4, SERP_TX_ST_REG_o(x1)       // read status of transmitter
    andi x4, x4, SERP_TX_ST_REG_READY_m // mask ready bit
    beq  x4, zero, tx_busy              // if not ready wait for ready condition
    sw   x3, SERP_TX_DATA_REG_o(x1)     // write byte to Tx data register
    jal  zero, next_char                // unconditional branch to process next byte

end_char:
    ebreak // stop continuous execution, request developer interaction
    jal  zero, end_char


.org 0x400

text_1: 
.asciz  "Hello world.\n"    // store zero terminated ASCII text

// if whole source compile is OK the switch to core tab
#pragma qtrvsim tab core


