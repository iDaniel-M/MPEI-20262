

_start:
main:
      addi x1, zero, 0         
      
      addi x2, zero, n
      lw x2, 0(x2)
      
      addi x3, zero, sum
      lw x3, 0(x3)
      
      addi x4, zero, array    

      addi x5, x4, 0

      jal  zero, test          

body: lw x6, 0(x5)            
      add x3, x3, x6     

      addi x5, x5, 4     
      addi x1, x1, 1          
   
test: blt x1, x2, body       

      addi x7, zero, sum
      sw x3, 0(x7)
      ebreak                 

.org 0x240
n:     .word 4                # number of elements in n
array: .word 3, 4, 5, 6       # array to sum up
sum:   .word 0                # sum should finally be 18 == 0x12
