# ==============================================================================
# ARCHIVO MAKEFILE BARE-METAL (INDEPENDIENTE)
# Automatiza la compilación inyectando la librería matemática base (libgcc.a)
# ==============================================================================

# TARGET define el nombre base que tendrán los archivos de salida generados (.elf, .bin, .map)
TARGET = frecuencimetro

# SOURCES define la lista de archivos de código fuente escritos en C que contienen la lógica
SOURCES = main.c 

# CC es el compilador cruzado de GNU (GCC) específico para procesadores ARM sin sistema operativo (none-eabi)
CC = arm-none-eabi-gcc

# OBJCOPY es la herramienta que extrae secciones específicas del ejecutable complejo (ELF) para crear el binario crudo
OBJCOPY = arm-none-eabi-objcopy

# SIZE es la utilidad que calcula los bytes consumidos en la memoria Flash (sección .text) y RAM (secciones .data y .bss)
SIZE = arm-none-eabi-size

# BANDERAS DE COMPILACIÓN (CFLAGS)
# -mcpu=cortex-m4 : Fija el conjunto de instrucciones físicas correspondientes al núcleo Cortex-M4.
# -mthumb         : Fuerza el uso exclusivo del conjunto de instrucciones Thumb-2 de 16/32 bits.
# -O2             : Aplica el nivel de optimización 2; reordena bucles y variables para máxima velocidad de ejecución.
# -Wall           : Activa todas las advertencias sintácticas y lógicas para evitar comportamientos indefinidos.
CFLAGS = -mcpu=cortex-m4 -mthumb -O2 -Wall

# RUTA DE LIBRERÍA MATEMÁTICA (LIBGCC)
# Pide al compilador que devuelva la ruta física exacta donde guarda sus rutinas de ensamblador como __aeabi_uldivmod.
LIBGCC = $(shell $(CC) $(CFLAGS) -print-libgcc-file-name)

# BANDERAS DEL ENLAZADOR (LDFLAGS)
# -T linker.ld       : Le indica explícitamente a GCC que use tu archivo llamado 'linker.ld' como mapa físico de memoria.
# -Wl,-Map=$(TARGET).map : Transmite el argumento '-Map' al enlazador interno para generar un archivo de texto con las direcciones de cada variable.
# --specs=nosys.specs: Desactiva las llamadas al sistema operativo anfitrión (evita errores buscando librerías de Linux/Windows).
# -nostdlib          : Previene la inyección automática de la biblioteca estándar de C genérica, reduciendo el tamaño del binario final.
LDFLAGS = -T linker.ld -Wl,-Map=$(TARGET).map --specs=nosys.specs -nostdlib

# Regla 'all': Es la meta predeterminada. Depende de que se construyan el ELF, el BIN y se ejecute print_size secuencialmente.
all: $(TARGET).elf $(TARGET).bin print_size

# Regla de construcción del ejecutable principal (ELF)
# Depende de los archivos listados en $(SOURCES). 
# $@ se sustituye automáticamente por el objetivo (frecuencimetro.elf).
# Se adjunta explícitamente $(LIBGCC) al final del comando para extraer el algoritmo de división de 64 bits.
$(TARGET).elf: $(SOURCES)
	$(CC) $(CFLAGS) $(SOURCES) $(LDFLAGS) -o $@ $(LIBGCC)

# Regla de extracción del binario puro (BIN)
# $< se sustituye por la primera dependencia (frecuencimetro.elf) y $@ por el objetivo (frecuencimetro.bin).
$(TARGET).bin: $(TARGET).elf
	$(OBJCOPY) -O binary $< $@

# Regla de análisis de memoria
# Imprime un cuadro resumen con las columnas text (Flash), data (RAM inicializada) y bss (RAM en cero).
print_size: $(TARGET).elf
	$(SIZE) $<

# Regla de limpieza (clean)
# Borra forzosamente (-f) los archivos residuales de compilaciones previas para empezar desde cero.
clean:
	rm -f $(TARGET).elf $(TARGET).bin $(TARGET).map

# Regla de flasheo a hardware (flash)
# Usa la herramienta st-link para inyectar el archivo binario a partir de la dirección base de la Flash.
flash: $(TARGET).bin
	st-flash write $(TARGET).bin 0x08000000