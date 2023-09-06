<!-- Please do not change this logo with link -->

[![MCHP](images/microchip.png)](https://www.microchip.com)

# PetiteFs on a Memory Card with PIC18F56Q71

[PetiteFs](http://elm-chan.org/fsw/ff/00index_e.html) is a lightweight, open-source library for FAT file system management. This example implements a memory card interface and implements the PetiteFs library to access the FAT file system on the card with the PIC18F56Q71 microcontroller. 

## Related Documentation

- [PetiteFs API Documentation](http://elm-chan.org/fsw/ff/00index_p.html)
- [FatFs Documentation](http://elm-chan.org/fsw/ff/00index_e.html)
- [Memory Card Communication](http://elm-chan.org/docs/mmc/mmc_e.html)

## Software Used

- [MPLAB&reg; X IDE v6.1.5 or newer](https://www.microchip.com/en-us/tools-resources/develop/mplab-x-ide?utm_source=GitHub&utm_medium=TextLink&utm_campaign=MCU8_MMTCha_pic18q71&utm_content=pic18f56q71-lw-memory-card-mplab-mcc&utm_bu=MCU08)
- [MPLAB XC8 v2.45 or newer](https://www.microchip.com/en-us/tools-resources/develop/mplab-xc-compilers?utm_source=GitHub&utm_medium=TextLink&utm_campaign=MCU8_MMTCha_pic18q71&utm_content=pic18f56q71-lw-memory-card-mplab-mcc&utm_bu=MCU08)
- [MPLAB Code Configurator (MCC)](https://www.microchip.com/en-us/tools-resources/configure/mplab-code-configurator?utm_source=GitHub&utm_medium=TextLink&utm_campaign=MCU8_MMTCha_pic18q71&utm_content=pic18f56q71-lw-memory-card-mplab-mcc&utm_bu=MCU08)
- [PetiteFs R0.03a (Included in Project)](http://elm-chan.org/fsw/ff/00index_p.html)

## Hardware Used

- [PIC18F56Q71  Curiosity Nano Evaluation Kit (EV01G21A)](https://www.microchip.com/en-us/development-tool/EV01G21A?utm_source=GitHub&utm_medium=TextLink&utm_campaign=MCU8_MMTCha_pic18q71&utm_content=pic18f56q71-lw-memory-card-mplab-mcc&utm_bu=MCU08)
- [Curiosity Nano Base for Click Boards&trade;](https://www.microchip.com/en-us/development-tool/AC164162?utm_source=GitHub&utm_medium=TextLink&utm_campaign=MCU8_MMTCha_pic18q71&utm_content=pic18f56q71-lw-memory-card-mplab-mcc&utm_bu=MCU08)
- [MicroSD Click (MIKROE-924)](https://www.mikroe.com/microsd-click)
- [2GB Memory Card](https://www.amazon.com/Transcend-microSD-Without-Adapter-TS2GUSDC/dp/B001BNNZXO/)

**Important: Memory Card must be 2 GB or less!**

## Setup

### Hardware Setup

With the power off, plug in the Curiosity Nano into the adapter board. Put the MicroSD Click in slot 1.

### Memory Card Setup

Before use, format the memory card as a FAT volume. Create a file called "test.txt" on the disk. In the file, type the following message:

`Hello from my Computer!`

A pre-formatted file is included inside the repository as well. **Important: PetiteFs cannot resize or create a file.** This means the file needs to be large enough to contain the full text written at runtime.

*Note: This example supports hot-swapping memory cards, and does not require the card to be installed on power-up.*

### UART Setup

1. Plug-in the Curiosity Nano.
2. Open MPLAB X IDE.
3. Click on the MPLAB Data Visualizer button in the toolbar.  
![Toolbar](./images/toolbar.png)
4. On the COM port associated with the nano, press the gear symbol.  
![COM Port](./images/comPort.png)
5. Set the baud rate to 115,200.
6. Click the terminal button to connect the COM port to the serial terminal.

Data will print when a memory card is inserted, as shown below.  
![Example Output](./images/exampleOutput.png)

## Implementing PetiteFs

PetiteFs handles file system management, but does not handle communication with the memory card. The library requires 3 functions to be implemented:

- `disk_initialize`
- `disk_readp`
- `disk_writep`

These functions call the memory card API to perform file system tasks.

**Important: This example has implemented these functions!**

## Operation

When a memory card is inserted, a switch in the socket pulls a detection line low. The microcontroller debounces this signal, then sets a flag to initialize the memory card outside of the interrupt handler. When inserted, the card may fail to initialize due to powering on, but the program will retry multiple times before erroring out. 

Communication with the memory card is via SPI. A series of commands are sent to the card to configure it for PetiteFs. For commands, the clock frequency is 400 kHz. During memory read/write, the clock frequency is increased up-to a maximum of 10.6 MHz, depending on the detected memory card support.

During normal operation, the memory card API maintains a cache of the current sector to improve performance of PetiteFs.

## Program Options

| Macro | Value | Description
| ----- | ----- | -----------
| MEM_CARD_DEBUG_ENABLE | Defined | Prints debug messages.
| MEM_CARD_MEMORY_DEBUG_ENABLE | Not defined | Prints the raw memory bytes received from the memory card.
| MEM_CARD_DISABLE_CACHE | Not defined | Disables file system caching. Used for debugging only.
| R1_TIMEOUT_BYTES | 10 | How many bytes to wait for a valid response code.
| READ_TIMEOUT_BYTES | 30 | How many bytes to wait for a data response.
| WRITE_TIMEOUT_BYTES | 30 | How many bytes to wait for a write response.
| INIT_RETRIES | 100 | How many times to attempt to send the initization command the memory card.
| FULL_RETRIES | 5 | If the all of the initialization commands, this the number of full retries before erroring out.
| DISABLE_SPEED_SWITCH | Not defined | If defined, the card does not shift the clock speed during read/write operations. 

*Note: PetiteFs has a set of macros to modify functionality and/or reduce memory usage. See `pffconf.h` for more information.*

## Summary

This example has demonstrated how to implement PetiteFs on the PIC18F56Q71. 