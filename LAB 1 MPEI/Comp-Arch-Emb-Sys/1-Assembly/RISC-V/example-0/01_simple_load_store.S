// Directives to make interesting windows visible
#pragma qtrvsim show registers
#pragma qtrvsim show memory


// label pointing to the first instruction that gets executed (0x200 in qtrvsim)
_start:
  
  lw    x1, 0x220(x0)
  lw    x2, 0x224(x0)
  add   x3, x1,x2
  	
  sw    x3, 0x228(x0)

  ebreak


.org 0x220
.word 0xDEADBEEF
.word 0x9E8D3D3C
.word 0xCAFE