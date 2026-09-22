/* C-Runtime Zero: Archivo de ensamblador de arranque responsable de acondicionar la CPU 
   antes de invocar el código principal en lenguaje C (Configuración del bare-metal) */

.syntax unified
.cpu cortex-m4
.thumb

/* Instruimos al Linker exportar estas subrutinas para que se liguen correctamente */
.global g_pfnVectors
.global Reset_Handler
.global _infinite_loop

/* 1. SECCIÓN VECTORIAL (Dirección crítica en la Memoria Flash 0x08000000) */
/* El STM32 requiere obligatoriamente que sus primeros bloques definan el puntero y a donde saltar al arrancar */
.section .isr_vector, "a", %progbits
g_pfnVectors:
    .word _estack            /* 0x00: Definición del puntero final de la SRAM (Stack Pointer Superior) */
    .word Reset_Handler      /* 0x04: Manejador de evento Reset (Dirección al encender el MCU) */

/* 2. CÓDIGO DE INICIO E INICIALIZACIÓN BÁSICA DE MEMORIA */
.section .text.Reset_Handler
.type Reset_Handler, %function
Reset_Handler:
    /* Paso A: Cargar el Stack Pointer de manera explícita por si el hardware fallara en autohacerlo */
    LDR R0, =_estack
    MOV SP, R0

    /* Paso B: Traspaso del segmento ".data"
       C toma el bloque de variables globales inicializadas en la Flash y las pasa a la RAM operativa */
    LDR R0, =_sdata   /* RAM: Puntero de Destino de escritura */
    LDR R1, =_edata   /* RAM: Límite final de escritura */
    LDR R2, =_sidata  /* FLASH: Puntero del Origen de lectura (donde quedó tras compilarse) */
    B ChequeoCopiaDatos /* Salto condicional al bucle */

BucleCopiaDatos:
    LDR R3, [R2], #4  /* Extrae un bloque de 4 bytes desde la Flash y avanza cursor */
    STR R3, [R0], #4  /* Inyecta el bloque de 4 bytes hacia la RAM y avanza cursor */

ChequeoCopiaDatos:
    CMP R0, R1        /* Verifica si ya alcanzó el fin de la memoria que necesitaba copiar */
    BLT BucleCopiaDatos /* De lo contrario, cicla de nuevo al subproceso BucleCopiaDatos */

    /* Paso C: Purgado y limpieza del segmento ".bss"
       Asegurarse de que las variables globales vacías o iniciadas en cero realmente estén en ceros */
    LDR R0, =_sbss
    LDR R1, =_ebss
    MOV R2, #0        /* Configura el registro R2 en cero constante */
    B ChequeoRellenoCeros

BucleRellenarCeros:
    STR R2, [R0], #4  /* Escribe un cero en memoria RAM y avanza 4 bytes de una vez */

ChequeoRellenoCeros:
    CMP R0, R1        /* Chequea si el proceso de borrado ya llegó a la dirección máxima estipulada */
    BLT BucleRellenarCeros /* Retorna al ciclo de borrado */

    /* Paso D: Iniciar Ejecución en C. La RAM ya está preparada y la pila conectada. */
    BL main           /* Branch with Link: Inyección en el archivo main.c de alto nivel */

/* Fallback de error grave: si algún error de hardware causara que main terminara 
   (algo prohibido en embebidos), forzamos un loop muerto infinito. */
.type _infinite_loop, %function
_infinite_loop:
    B _infinite_loop