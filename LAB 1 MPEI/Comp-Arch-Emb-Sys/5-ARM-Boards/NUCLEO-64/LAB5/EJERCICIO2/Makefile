# Nombre del archivo ejecutable de salida
TARGET = main

# Herramientas del toolchain GNU para ARM Bare-Metal
CC      = arm-none-eabi-gcc
OBJCOPY = arm-none-eabi-objcopy
SIZE    = arm-none-eabi-size

# Archivos fuente (.c) dentro de la carpeta local
SRCS = main.c

# Objetos generados en la compilación
OBJS = $(SRCS:.c=.o)

# Incluir únicamente la carpeta local
INC_DIRS = -I.

# Parámetros del microcontrolador Cortex-M4 con FPU Hard
MCU = -mcpu=cortex-m4 -mthumb -mfpu=fpv4-sp-d16 -mfloat-abi=hard

# Parámetros de compilación (Búsqueda local de cabeceras)
CFLAGS = $(MCU) $(INC_DIRS) -O0 -g3 -Wall -ffunction-sections -fdata-sections

# Parámetros de enlace con el Linker Script local
LDFLAGS = $(MCU) -Tlinker.ld -Wl,--gc-sections -Wl,-Map=$(TARGET).map --specs=nano.specs

# Regla principal por defecto
all: $(TARGET).elf $(TARGET).bin print_size

# Compilación de archivos de C a código objeto .o
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Enlazado final para obtener el binario .elf
$(TARGET).elf: $(OBJS)
	$(CC) $(OBJS) $(LDFLAGS) -o $@

# Extracción a formato binario puro para carga directa en Flash (.bin)
$(TARGET).bin: $(TARGET).elf
	$(OBJCOPY) -O binary $< $@

# Muestra el consumo exacto de memoria Flash y RAM en terminal
print_size: $(TARGET).elf
	$(SIZE) $<

# Flasheo directo hacia la placa a través del ST-LINK con OpenOCD
flash: $(TARGET).elf
	openocd -f interface/stlink.cfg -f target/stm32f4x.cfg -c "program $(TARGET).elf verify reset exit"

# Limpieza de archivos compilados
clean:
	rm -f *.o *.elf *.bin *.map

.PHONY: all clean flash print_size