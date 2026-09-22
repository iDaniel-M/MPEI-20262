/* C-Runtime Zero: Archivo de ensamblador de arranque responsable de acondicionar la CPU 
   antes de invocar el codigo principal en lenguaje C (Configuracion del bare-metal) */

.syntax unified
.cpu cortex-m4
.thumb

/* Instruimos al Linker exportar estas subrutinas para que se liguen correctamente */
.global g_pfnVectors
.global Reset_Handler
.global _infinite_loop

/* 1. SECCIoN VECTORIAL (Direccion importante en la Memoria Flash 0x08000000) */
/* El STM32 requiere obligatoriamente que sus primeros bloques definan el puntero y a donde saltar al arrancar */
.section .isr_vector, "a", %progbits
g_pfnVectors:
    .word _estack            /* 0x00: Definicion del puntero final de la SRAM (Stack Pointer Superior) */
    .word Reset_Handler      /* 0x04: Manejador de evento Reset (Direccion al encender el MCU) */

/* 2. CODIGO DE INICIO E INICIALIZACION BASICA DE MEMORIA */
.section .text.Reset_Handler
.type Reset_Handler, %function
Reset_Handler:
    /* Paso A: Cargar el Stack Pointer de manera explicita por si el hardware fallara en autohacerlo */
    LDR R0, =_estack
    MOV SP, R0

    /* Paso B: Traspaso del segmento ".data"
       C toma el bloque de variables globales inicializadas en la Flash y las pasa a la RAM operativa */
    LDR R0, =_sdata   /* RAM: Puntero de Destino de escritura */
    LDR R1, =_edata   /* RAM: Limite final de escritura */
    LDR R2, =_sidata  /* FLASH: Puntero del Origen de lectura (donde quedo tras compilarse) */
    B ChequeoCopiaDatos /* Salto condicional al bucle */

BucleCopiaDatos:
    LDR R3, [R2], #4  /* Extrae un bloque de 4 bytes desde la Flash y avanza cursor */
    STR R3, [R0], #4  /* Inyecta el bloque de 4 bytes hacia la RAM y avanza cursor */

ChequeoCopiaDatos:
    CMP R0, R1        /* Verifica si ya alcanzo el fin de la memoria que necesitaba copiar */
    BLT BucleCopiaDatos /* De lo contrario, cicla de nuevo al subproceso BucleCopiaDatos */

    /* Paso C: Purgado y limpieza del segmento ".bss"
       Asegurarse de que las variables globales vacias o iniciadas en cero realmente esten en ceros */
    LDR R0, =_sbss
    LDR R1, =_ebss
    MOV R2, #0        /* Configura el registro R2 en cero constante */
    B ChequeoRellenoCeros

BucleRellenarCeros:
    STR R2, [R0], #4  /* Escribe un cero en memoria RAM y avanza 4 bytes de una vez */

ChequeoRellenoCeros:
    CMP R0, R1        /* Chequea si el proceso de borrado ya llego a la direccion maxima estipulada */
    BLT BucleRellenarCeros /* Retorna al ciclo de borrado */

    /* Paso D: Iniciar Ejecucion en C. La RAM ya esta preparada y la pila conectada. */
    BL main           /* Branch with Link: Inyeccion en el archivo main.c de alto nivel */

/* Si algun error de hardware causara que main terminara 
   (algo prohibido en embebidos), forzamos un loop muerto infinito. */
.type _infinite_loop, %function
_infinite_loop:
    B _infinite_loop