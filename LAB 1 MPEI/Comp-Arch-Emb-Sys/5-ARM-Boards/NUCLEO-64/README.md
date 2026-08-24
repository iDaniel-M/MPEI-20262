# Baremetal examples on NUCLEO-64 Board

This examples were created to work on the NUCLEO-64 board, however they can be adapted to work on the NUCLEO-144 or any other similar STM32F4xx device. 

### [1-led-blink](./1-led-blink/)
This is a basic Bare-metal of "frestanding" example that blinks a LED on the NUCLEO 64 board using the STM32F401RE microcontroller. The Program directly reads/writes the peripheral memory space to configure the GPIO and required peripherals. The blink interval was achieve by making the CPU spin in a long loop waiting for some time to change the LED status.

### [2-led-blink](./2-led-blink/)
This example does the same as the previous example, however the time between LED transitions is dinamycally adapted by modifying the leng of the delay loop. 

### [3-input-butt](./3-input-butt/)
Using the same programing approach, this example configures the LED (PA5) and the PushButton (PC13) by writing directly on the register addresses. The program reads the status of the BUtton (PC13), when the button is HIGH, the program turns on the LED. When the Button is LOW the program Turns off the LED. 

### [4-custom-hal](./4-custom-hal/)
This example shows how to organize the program using helper functions (APIs) and register definitions using structures in C language. The example shows how to create a basic HAL (Hardware Abstraction Library) for configure and access the GPIO in a friendly manner (e.g., [gpio_config.h](./4-custom-hal/inc/gpio_config.h) and [gpio_config.c](./4-custom-hal/src/gpio_config.c)). In addition, a header file [stm32f401.h](./4-custom-hal/inc/stm32f401.h) contains the register defintions for the GPIOs and the RCC required during the exmaple's configuration. 

The program works same as the example [3-input-butt](./3-input-butt/), but now the [main.c](./4-custom-hal/main.c) file is cleaner. 

### [5-systick-delay](./5-systick-delay)
This example shows how to use the SysTick timer in ARM systems to create delay functions in a more precise manner than just using loops. It is worth noting that the delay functions created in this example are blocking functions, because they are still keeping the CPU waiting for the delay time. 

This excercise blinks the LED (PA5) at diferent rates according to the status of a Button (PC13). When the Button is HIGH the LED blinks every 500ms (500000 us); when the Button is LOW the LED blinks every 250ms (250000us). 

### [6-CMSIS-example](./6-CMSIS-example/)
ARM Provides a set of libraries and helper functions for their processors, they are called CMSIS. In this example we are using the basic CMSIS libraries along the device specific STM32F4xx definitions.  This example implements the same logic as the example ([5-systick-delay](./5-systick-delay/)), but now using the definitions provided by the manufacturer under the CMSIS libraries for ARM. In order to make it work there were some adjustements in the example, including the [linkerscript](./6-CMSIS-example/linker_script.ld) and the [Makefile](./6-CMSIS-example/Makefile). 

### [7-config-clock](./7-config-clock/)
This example does the same as the example ([6-CMSIS-example](./6-CMSIS-example/)), however this time the clock tree was configured to increase the clock frequency of the CPU to 84MHz. It uses the internal oscilator (HSI) that works at 16MHz and configures the PLL to increase it upt to 84MHz. At the same time it sets the USB frequceny to 48MHz, even though we are not using the USB peripherals. 

We put together the SysTick configuration, the clock configuration and the delays under the same HAL named system_config (i.e., [system_config.c](./7-config-clock/src/system_config.c) and [/system_config.h](./7-config-clock/inc/system_config.h))

Now we can configure our system to works either to 16MHs (default) or 84MHz based on the definition available inside the [/system_config.h](./7-config-clock/inc/system_config.h) file `#define SYCLK 84000000` you can change the value between 84000000 or 16000000. 


### [8-STM32-HAL](./8-STM32-HAL/)
This example shows how to use the predefined HAL libraries provided by the device vendor. This is a showcase of how to configure the GPIO and SysTick timer to implement the the same behaviour as the two previous examples; blink a LED on different rates based on the state of an input Button. It is worth noting that you need to add a HAL configuration header [stm32f4xx_hal_conf.h](./8-STM32-HAL/inc/stm32f4xx_hal_conf.h) which allows to select specific HALs in your project.

To be able of use the HAL APIs you need to check the HAL documentation available [here](https://www.st.com/resource/en/user_manual/um1725-description-of-stm32f4-hal-and-lowlayer-drivers-stmicroelectronics.pdf). The HAL helps with the usage of peripherals using high level functions and configuration structures w/o checking every configuration bit of the registers. 

In order to use the HAL libraries the linkerscript and Makefile were adjusted in order to include and compile the neceary files asociated to them. 

### [9-Interrupts-SysTick](./9-Interrupts-SysTick/)
This example Shows how to use the SysTick to rise an interrupt every 1ms. The example shows two ways of configuring the SysTick timer: using a custom configuration functions or using the HAL. The program blinks the LED connected to the PA5 based on the status of the input Button connected to the PC13. The program does not use delays, allowing the procesor to execute other things while the required time happens. 

Notice that we created an structure that allocates two timers via software based on the 1ms timer interrupt, this allows us to have handle tasks using different timing events.


### [10-Interrupts-EXT](./10-Interrupts-EXT/)
This example showcases the use of external interrupts, configuring the PC13 as input and enabling interrupts at every rising edge change on tat pin. The example still implements the same blinking LED example as previous exercises. but this time when the user release the button the LED blink rates changes. This example has enabled two interrupts: SysTick IRQ and the EXTI15_10 IRQ. 

Unlike the SysTick, the EXTI Pending Flag must be cleared, otherwise the NVIC controller will cause to enter continously to the EXT IRQ handler preventing the execution of other parts of the code.

### [11-Interrupts-Timers]()

### [12-USART]()

### [13-SPI]()

### [14-I2C]()

### [15-ADC]()

### [16-PWM]()

### [17-DAC]()

### [18-Power-Magement]()

### [19-RTOS]()

### [20-Micropython]()



