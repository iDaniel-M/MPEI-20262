# Define la cadena de compilacion de GCC especializada en arquitecturas ARM que no tienen OS subyacente
CC = arm-none-eabi-gcc

# Banderas de compilador detalladas para C:
# -Wall -Wextra: Habilita chequeos estrictos del compilador para evitar fallos logicos 
# -std=c11: Fija el estandar sintactico a C de 2011
# -ffreestanding: Indica que el entorno no tiene biblioteca C estandar (stdlib) montada
# -Og -g: Orientado a depuracion (g) con minima optimizacion (Og)
# -mcpu=cortex-m4: Afirma la arquitectura del procesador del STM32 Nucleo
# -mthumb: Obliga al procesador a decodificar instrucciones en conjunto Thumb-2 (Ahorro de flash)
# -mfpu / -mfloat-abi: Habilita el coprocesador de punto flotante de hardware
CFLAGS = -Wall -Wextra -std=c11 -ffreestanding -Og -g -mcpu=cortex-m4 -mthumb -mfpu=fpv4-sp-d16 -mfloat-abi=hard -Iinc -nostdlib

# Banderas de enlazado (Linker) indicando que use nuestro linker script de memoria propio 
LDFLAGS = -nostdlib -Tlinker_script.ld -Wl,--no-relax,-Map=juego_triqui.map

# Compilacion de los archivos intermedios generados para la union posterior
OBJS = src/main.o src/gpio_config.o src/game.o crt0.o

# Reglas principales
all: juego_triqui.elf juego_triqui.bin

# Receta de Make generica que indica como transformar cada *.c en un *.o
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Receta de Make generica para compilar archivos de ensamblador (*.S)
%.o: %.S
	$(CC) $(CFLAGS) -c $< -o $@

# Tarea de enlazado: mezcla los *.o siguiendo el script .ld y arroja el binario .elf
juego_triqui.elf: $(OBJS) linker_script.ld
	$(CC) $(LDFLAGS) $(OBJS) -o juego_triqui.elf

# Tarea de volcado: traduce el ELF en formato binario crudo (.bin) capaz de copiarse al micro
juego_triqui.bin: juego_triqui.elf
	arm-none-eabi-objcopy -O binary juego_triqui.elf juego_triqui.bin

# Tarea interactiva que lanza el CLI oficial de ST y quema la memoria flash en el banco 0x08000000 
flash: juego_triqui.bin
	/mnt/d/Uni/MPEI/STM32/bin/STM32_Programmer_CLI.exe -c port=usb1 -w juego_triqui.bin 0x08000000 -v

# Tarea de limpieza que remueve todos los objetos y compilados generados
clean:
	rm -f src/*.o crt0.o juego_triqui.elf juego_triqui.bin juego_triqui.map