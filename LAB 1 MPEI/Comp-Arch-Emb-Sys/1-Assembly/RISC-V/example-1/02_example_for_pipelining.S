// Directives to make interesting windows visible
#pragma qtrvsim show registers
#pragma qtrvsim show memory


// label pointing to the first instruction that gets executed (0x200 in qtrvsim)
_start:
  
  lw     x1, 0x220(x0)
  lw     x2, 0x224(x0)
  add x3, x1,x2
  or    x4, x1,x3
	
  sw     x3, 0x228(x0)

  ebreak

//  sw    x3, 0x218(x0)

.org 0x220
.word 0x1 //DEADBEEF
.word 0x2  //CAFE
