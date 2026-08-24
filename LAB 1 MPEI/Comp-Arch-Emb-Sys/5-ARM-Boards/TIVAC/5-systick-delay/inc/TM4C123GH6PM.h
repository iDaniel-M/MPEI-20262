#ifndef __TM4C123GH6PM_H__
#define __TM4C123GH6PM_H__
#include <stdint.h>


#define PERIPHERALS_BASE 0x40000000u

#define SYSCTL_BASE (PERIPHERALS_BASE + 0xFE000u)
#define GPIOA_BASE  (PERIPHERALS_BASE + 0x04000u)
#define GPIOB_BASE  (PERIPHERALS_BASE + 0x05000u)
#define GPIOC_BASE  (PERIPHERALS_BASE + 0x06000u)
#define GPIOD_BASE  (PERIPHERALS_BASE + 0x07000u)
#define GPIOE_BASE  (PERIPHERALS_BASE + 0x24000u)
#define GPIOF_BASE  (PERIPHERALS_BASE + 0x25000u)


typedef struct {
    volatile uint32_t DID0;         // 0x000 Device Identification 0
    volatile uint32_t DID1;         // 0x004 Device Identification 1
    volatile uint32_t DC0;          // 0x008 Device Capabilities 0
    uint32_t reserved0[1];         // 0x00C Reserved space
    volatile uint32_t DC1;          // 0x010 Device Capabilities 1
    volatile uint32_t DC2;          // 0x014 Device Capabilities 2
    volatile uint32_t DC3;          // 0x018 Device Capabilities 3
    volatile uint32_t DC4;          // 0x01C Device Capabilities 4
    volatile uint32_t DC5;          // 0x020 Device Capabilities 5
    volatile uint32_t DC6;          // 0x024 Device Capabilities 6
    volatile uint32_t DC7;          // 0x028 Device Capabilities 7
    volatile uint32_t DC8;          // 0x02C Device Capabilities 8
    volatile uint32_t PBORCTL;      // 0x030 Brown-Out Reset Control
    uint32_t reserved1[3];          // 0x034 Reserved space
    volatile uint32_t SRCR0;        // 0x040 Software Reset Control 0
    volatile uint32_t SRCR1;        // 0x044 Software Reset Control 1
    volatile uint32_t SRCR2;        // 0x048 Software Reset Control 2
    uint32_t reservedx1[1];         // 0x04C Reserved space
    volatile uint32_t RIS;          // 0x050 Raw Interrupt Status
    volatile uint32_t IMC;          // 0x054 Interrupt Mask Control
    volatile uint32_t MISC;         // 0x058 Masked Interrupt Status and Clear
    volatile uint32_t RESC;         // 0x05C Reset Cause
    volatile uint32_t RCC;          // 0x060 Run-Mode Clock Configuration
    uint32_t reserved2[2];         // 0x064 Reserved space
    volatile uint32_t GPIOHBCTL;    // 0x06C GPIO High-Performance Bus Control
    volatile uint32_t RCC2;         // 0x070 Run-Mode Clock Configuration 2
    uint32_t reserved3[2];         // 0x074 Reserved space
    volatile uint32_t MOSCCTL;      // 0x07C Main Oscillator Control
    uint32_t reserved4[49];        // 0x080 Reserved space
    volatile uint32_t DSLPCLKCFG;   // 0x144 Deep Sleep Clock Configuration
    uint32_t reserved5[1];         // 0x148 Reserved space
    volatile uint32_t SYSPROP;      // 0x14C System Properties
    volatile uint32_t PIOSCCAL;     // 0x150 Precision Internal Oscillator Calibration
    volatile uint32_t PIOSCSTAT;    // 0x154 Precision Internal Oscillator Statistics
    uint32_t reserved6[2];         // 0x158 Reserved space
    volatile uint32_t PLLFREQ0;     // 0x160 PLL Frequency 0
    volatile uint32_t PLLFREQ1;     // 0x164 PLL Frequency 1
    volatile uint32_t PLLSTAT;      // 0x168 PLL Status
    uint32_t reserved7[7];         // 0x16C Reserved space
    volatile uint32_t SLPPWRCFG;    // 0x188 Sleep Power Configuration
    volatile uint32_t DSLPPWRCFG;   // 0x18C Deep Sleep Power Configuration
    uint32_t reserved8[9];         // 0x190 Reserved space
    volatile uint32_t LDOSPCTL;     // 0x1B4 LDO Sleep Power Control
    volatile uint32_t LDOSPCAL;     // 0x1B8 LDO Deep-Sleep Power Calibration
    volatile uint32_t LDODPCTL;     // 0x1BC LDO Deep-Sleep Power Status
    volatile uint32_t LDODPCAL;     // 0x1C0 LDO Deep-Sleep Power Calibration
    uint32_t reserved9[2];         // 0x1C4 Reserved space
    volatile uint32_t SDPMST;       // 0x1CC Sleep Deep Power Mode Status
    uint32_t reserved10[76];        // 0x1D0 Reserved space
    volatile uint32_t PPWD;         // 0x300 Watchdog Timer Peripheral Present
    volatile uint32_t PPTIMER;      // 0x304 Timer Peripheral Present
    volatile uint32_t PPGPIO;       // 0x308 General-Purpose Input/Output Peripheral Present
    volatile uint32_t PPDMA;        // 0x30C Micro Direct Memory Access Peripheral Present
    uint32_t reserved11[1];          // 0x310 Reserved space
    volatile uint32_t PPHIB;        // 0x314 Hibernation Peripheral Present
    volatile uint32_t PPUART;       // 0x318 Universal Asynchronous Receiver/Transmitter Peripheral Present
    volatile uint32_t PPSSI;        // 0x31C Synchronous Serial Interface Peripheral Present
    volatile uint32_t PPI2C;        // 0x320 Inter-Integrated Circuit Peripheral Present
    uint32_t reserved12[1]; 
    volatile uint32_t PPUSB;        // 0x328 Universal Serial Bus Peripheral Present
    uint32_t reserved13[2];         // 0x32C
    volatile uint32_t PPCAN;        // 0x334 Controller Area Network Peripheral Present
    volatile uint32_t PPADC;        // 0x338 Analog-to-Digital Converter Peripheral Present
    volatile uint32_t PPACMP;       // 0x33C Analog Comparator Peripheral Present
    volatile uint32_t PPPWM;        // 0x340 Pulse Width Modulator Peripheral Present
    volatile uint32_t PPQEI;        // 0x344 Quadrature Encoder Interface Peripheral Present
    uint32_t reserved14[4];         // Reserved space
    volatile uint32_t PPEEPROM;     // 0x358 EEPROM Peripheral Present
    volatile uint32_t PPWTIMER;     // 0x35C Wide Timer Peripheral Present
    uint32_t reserved15[104];       // Reserved space
    volatile uint32_t SRWD;         // 0x500 Watchdog Timer Software Reset
    volatile uint32_t SRTIMER;      // 0x504 Timer Software Reset
    volatile uint32_t SRGPIO;       // 0x508 General-Purpose Input/Output Software Reset
    volatile uint32_t SRDMA;        // 0x50C Micro Direct Memory Access Software Reset
    uint32_t reserved16[1];       // Reserved space
    volatile uint32_t SRHIB;        // 0x514 Hibernation Software Reset
    volatile uint32_t SRUART;       // 0x518 Universal Asynchronous Receiver/Transmitter Software Reset
    volatile uint32_t SRSSI;        // 0x51C Synchronous Serial Interface Software Reset
    volatile uint32_t SRI2C;        // 0x520 Inter-Integrated Circuit Software Reset
    uint32_t reserved17[1];
    volatile uint32_t SRUSB;        // 0x528 Universal Serial Bus Software Reset
    uint32_t reserved18[2];
    volatile uint32_t SRCAN;        // 0x534 Controller Area Network Software Reset
    volatile uint32_t SRADC;        // 0x538 Analog-to-Digital Converter Software Reset
    volatile uint32_t SRACMP;       // 0x53C Analog Comparator Software Reset
    volatile uint32_t SRPWM;        // 0x540 Pulse Width Modulator Software Reset
    volatile uint32_t SRQEI;        // 0x544 Quadrature Encoder Interface Software Reset
    uint32_t reserved19[4];
    volatile uint32_t SREEPROM;     // 0x558 EEPROM Software Reset
    volatile uint32_t SRWTIMER;     // 0x55C Wide Timer Software Reset
    uint32_t reserved20[40];
    volatile uint32_t RCGCWD;       // 0x600 Watchdog Timer Run Mode Clock Gating Control
    volatile uint32_t RCGCTIMER;    // 0x604 Timer Run Mode Clock Gating Control
    volatile uint32_t RCGCGPIO;     // 0x608 General-Purpose Input/Output Run Mode Clock Gating Control
    volatile uint32_t RCGCDMA;      // 0x60C  Micro Direct Memory Access Run Mode Clock Gating Control
    uint32_t reserved21[1];
    volatile uint32_t RCGCHIB;      // 0x614 Hibernation Run Mode Clock Gating Control
    volatile uint32_t RCGCUART;     // 0x618 Universal Asynchronous Receiver/Transmitter Run Mode Clock Gating Control
    volatile uint32_t RCGCSSI;      // 0x61C Synchronous Serial Interface Run Mode Clock Gating Control
    volatile uint32_t RCGCI2C;      // 0x620 Inter-Integrated Circuit Run Mode Clock Gating Control
    uint32_t reserved22[1];         // 0x624
    volatile uint32_t RCGCUSB;      // 0x628 Universal Serial Bus Run Mode Clock Gating Control
    uint32_t reserved23[2];         // 0x62C
    volatile uint32_t RCGCCAN;      // 0x634 Controller Area Network Run Mode Clock Gating Control
    volatile uint32_t RCGCADC;      // 0x638 Analog-to-Digital Converter Run Mode Clock Gating Control
    volatile uint32_t RCGCACMP;     // 0x63C Analog Comparator Run Mode Clock Gating Control
    volatile uint32_t RCGCPWM;      // 0x640 Pulse Width Modulator Run Mode Clock Gating Control
    volatile uint32_t RCGCQEI;      // 0x644 Quadrature Encoder Interface Run Mode Clock Gating Control
    uint32_t reserved24[4];         // 0x648
    volatile uint32_t RCGCEEPROM;   // 0x658 EEPROM Run Mode Clock Gating Control
    volatile uint32_t RCGCWTIMER;   // 0x65C Wide Timer Run Mode Clock Gating Control
    uint32_t reserved25[40];
    volatile uint32_t SCGCWD;       //0x700 Watchdog Timer Sleep Mode Clock Gating Control
    volatile uint32_t SCGCTIMER;    // Timer Sleep Mode Clock Gating Control
    volatile uint32_t SCGCGPIO;     // General-Purpose Input/Output Sleep Mode Clock Gating Control
    volatile uint32_t SCGCDMA;      // Micro Direct Memory Access Sleep Mode Clock Gating Control
    uint32_t reserved26[1];
    volatile uint32_t SCGCHIB;      // Hibernation Sleep Mode Clock Gating Control
    volatile uint32_t SCGCUART;     // Universal Asynchronous Receiver/Transmitter Sleep Mode Clock Gating Control
    volatile uint32_t SCGCSSI;      // Synchronous Serial Interface Sleep Mode Clock Gating Control
    volatile uint32_t SCGCI2C;      // Inter-Integrated Circuit Sleep Mode Clock Gating Control
    uint32_t reserved27[1];
    volatile uint32_t SCGCUSB;      // Universal Serial Bus Sleep Mode Clock Gating Control
    uint32_t reserved28[2];
    volatile uint32_t SCGCCAN;      // Controller Area Network Sleep Mode Clock Gating Control
    volatile uint32_t SCGCADC;      // Analog-to-Digital Converter Sleep Mode Clock Gating Control
    volatile uint32_t SCGCACMP;     // Analog Comparator Sleep Mode Clock Gating Control
    volatile uint32_t SCGCPWM;      // Pulse Width Modulator Sleep Mode Clock Gating Control
    volatile uint32_t SCGCQEI;      // Quadrature Encoder Interface Sleep Mode Clock Gating Control
    uint32_t reserved29[4];
    volatile uint32_t SCGCEEPROM;   // EEPROM Sleep Mode Clock Gating Control
    volatile uint32_t SCGCWTIMER;   //0x75C Wide Timer Sleep Mode Clock Gating Control
    uint32_t reserved30[40];
    volatile uint32_t DCGCWD;       //0x800 Watchdog Timer Deep-Sleep Mode Clock Gating Control
    volatile uint32_t DCGCTIMER;    // Timer Deep-Sleep Mode Clock Gating Control
    volatile uint32_t DCGCGPIO;     // General-Purpose Input/Output Deep-Sleep Mode Clock Gating Control
    volatile uint32_t DCGCDMA;      // Micro Direct Memory Access Deep-Sleep Mode Clock Gating Control
    uint32_t reserved31[1];
    volatile uint32_t DCGCHIB;      // Hibernation Deep-Sleep Mode Clock Gating Control
    volatile uint32_t DCGCUART;     // Universal Asynchronous Receiver/Transmitter Deep-Sleep Mode Clock Gating Control
    volatile uint32_t DCGCSSI;      // Synchronous Serial Interface Deep-Sleep Mode Clock Gating Control
    volatile uint32_t DCGCI2C;      // Inter-Integrated Circuit Deep-Sleep Mode Clock Gating Control
    uint32_t reserved32[1];
    volatile uint32_t DCGCUSB;      // Universal Serial Bus Deep-Sleep Mode Clock Gating Control
    uint32_t reserved33[2];
    volatile uint32_t DCGCCAN;      // Controller Area Network Deep-Sleep Mode Clock Gating Control
    volatile uint32_t DCGCADC;      // Analog-to-Digital Converter Deep-Sleep Mode Clock Gating Control
    volatile uint32_t DCGCACMP;     // Analog Comparator Deep-Sleep Mode Clock Gating Control
    volatile uint32_t DCGCPWM;      // Pulse Width Modulator Deep-Sleep Mode Clock Gating Control    
    volatile uint32_t DCGCQEI;      // Quadrature Encoder Interface Deep-Sleep Mode Clock Gating Control
    uint32_t reserved34[4];
    volatile uint32_t DCGCEEPROM;   // EEPROM Deep-Sleep Mode Clock Gating Control
    volatile uint32_t DCGCWTIMER;   //0x85C Wide Timer Deep-Sleep Mode Clock Gating Control
    uint32_t reserved35[104];
    volatile uint32_t PRWD;         //0xA00 Watchdog Timer Peripheral Ready
    volatile uint32_t PRTIMER;      // Timer Peripheral Ready
    volatile uint32_t PRGPIO;       // General-Purpose Input/Output Peripheral Ready
    volatile uint32_t PRDMA;        // Micro Direct Memory Access Peripheral Ready
    uint32_t reserved36[1];       // Reserved space
    volatile uint32_t PRHIB;        // Hibernation Peripheral Ready
    volatile uint32_t PRUART;       // Universal Asynchronous Receiver/Transmitter Peripheral Ready
    volatile uint32_t PRSSI;        // Synchronous Serial Interface Peripheral Ready
    volatile uint32_t PRI2C;        // Inter-Integrated Circuit Peripheral Ready
    uint32_t reserved37[1];
    volatile uint32_t PRUSB;        // Universal Serial Bus Peripheral Ready
    uint32_t reserved38[2];
    volatile uint32_t PRCAN;        // Controller Area Network Peripheral Ready
    volatile uint32_t PRADC;        // Analog-to-Digital Converter Peripheral Ready
    volatile uint32_t PRACMP;       // Analog Comparator Peripheral Ready
    volatile uint32_t PRPWM;        // Pulse Width Modulator Peripheral Ready
    volatile uint32_t PRQEI;        // Quadrature Encoder Interface Peripheral Ready
    uint32_t reserved39[4];
    volatile uint32_t PREEPROM;     // EEPROM Peripheral Ready
    volatile uint32_t PRWTIMER;     // Wide Timer Peripheral Ready
} SYSCTL_TypeDef;

#define SYSCTL ((volatile  SYSCTL_TypeDef *) SYSCTL_BASE)


typedef struct {
    volatile uint32_t GPIODATA[256];    // 0x000 Data
    //uint32_t reserved0[255];          // 0x004 Reserved space
    volatile uint32_t GPIODIR;          // 0x400 Direction
    volatile uint32_t GPIOIS;           // 0x404 Interrupt Sense
    volatile uint32_t GPIOIBE;          // 0x408 Interrupt Both Edges
    volatile uint32_t GPIOIEV;          // 0x40C Interrupt Event
    volatile uint32_t GPIOIM;           // 0x410 Interrupt Mask
    volatile uint32_t GPIORIS;          // 0x414 Raw Interrupt Status
    volatile uint32_t GPIOMIS;          // 0x418 Masked Interrupt Status
    volatile uint32_t GPIOICR;          // 0x41C Interrupt Clear
    volatile uint32_t GPIOAFSEL;        // 0x420 Alternate Function Select
    uint32_t reserved1[55];         // 0x424 Reserved space
    volatile uint32_t GPIODR2R;         // 0x500 2-mA Drive Select
    volatile uint32_t GPIODR4R;         // 0x504 4-mA Drive Select
    volatile uint32_t GPIODR8R;         // 0x508 8-mA Drive Select
    volatile uint32_t GPIOODR;          // 0x50C Open Drain Select
    volatile uint32_t GPIOPUR;          // 0x510 Pull-Up Select
    volatile uint32_t GPIOPDR;          // 0x514 Pull-Down Select
    volatile uint32_t GPIOSLR;          // 0x518 Slew Rate Control Select
    volatile uint32_t GPIODEN;          // 0x51C Digital Enable
    volatile uint32_t GPIOLOCK;         // 0x520 Lock
    volatile uint32_t GPIOCR;           // 0x524 Commit
    volatile uint32_t GPIOAMSEL;        // 0x528 Analog Mode Select
    volatile uint32_t GPIOPCTL;         // 0x52C Port Control
    volatile uint32_t GPIOADCCTL;       // 0x530 ADC Control
    volatile uint32_t GPIODMACTL;       // 0x534 DMA Control
    uint32_t reserved2[678];            // 0x538 Reserved space
    volatile uint32_t GPIOPeriphID4;    // 0xFD0 Peripheral Identification 4
    volatile uint32_t GPIOPeriphID5;    // 0xFD4 Peripheral Identification 5
    volatile uint32_t GPIOPeriphID6;    // 0xFD8 Peripheral Identification 6
    volatile uint32_t GPIOPeriphID7;    // 0xFDC Peripheral Identification 7
    volatile uint32_t GPIOPeriphID0;    // 0xFE0 Peripheral Identification 0
    volatile uint32_t GPIOPeriphID1;    // 0xFE4 Peripheral Identification 1
    volatile uint32_t GPIOPeriphID2;    // 0xFE8 Peripheral Identification 2
    volatile uint32_t GPIOPeriphID3;    // 0xFEC Peripheral Identification 3
    volatile uint32_t GPIOPCellID0;     // 0xFF0 PrimeCell Identification 0
    volatile uint32_t GPIOPCellID1;     // 0xFF4 PrimeCell Identification 1
    volatile uint32_t GPIOPCellID2;     // 0xFF8 PrimeCell Identification 2
    volatile uint32_t GPIOPCellID3;     // 0xFFC PrimeCell Identification 3
} GPIO_TypeDef;

#define GPIOA ((volatile GPIO_TypeDef *) GPIOA_BASE)
#define GPIOB ((volatile GPIO_TypeDef *) GPIOB_BASE)
#define GPIOC ((volatile GPIO_TypeDef *) GPIOC_BASE)
#define GPIOD ((volatile GPIO_TypeDef *) GPIOD_BASE)
#define GPIOE ((volatile GPIO_TypeDef *) GPIOE_BASE)
#define GPIOF ((volatile GPIO_TypeDef *) GPIOF_BASE)

#define SYSPRV_BASE (0xE000E000U) 
#define SYSTICK_BASE (SYSPRV_BASE + 0x10U) // Peripheral base address for STM32F4 reference manual  pg. 38

typedef struct {
    volatile unsigned int STCSR; // SysTick Control and Status Register
    volatile unsigned int STRVR; // SysTick Reload Value Register
    volatile unsigned int STCVR; // SysTick Current Value Register
    volatile unsigned int STCR; // SysTick Calibration Value Register
} SysTick_TypeDef;

#define SysTick ((volatile SysTick_TypeDef *) SYSTICK_BASE)

#endif /* __TM4C123GH6PM_H__ */