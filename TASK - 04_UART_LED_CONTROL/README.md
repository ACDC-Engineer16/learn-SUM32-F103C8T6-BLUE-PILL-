# STM32 UART Controlled LED

## Overview
This project demonstrates how to control an LED using UART communication on an STM32 microcontroller using the HAL library.

The user sends commands via UART:
- `1` → Turn LED ON
- `0` → Turn LED OFF

The microcontroller responds back with status messages over UART.

---

## Features
- UART communication at **115200 baud**
- Simple command-based LED control
- Real-time user interaction via serial terminal
- Uses STM32 HAL library

---

## Hardware Requirements
- STM32 microcontroller board (e.g., STM32F1 / STM32F4 series)
- On-board or external LED connected to `LEDOUT_Pin`
- USB to UART interface (or onboard ST-Link Virtual COM Port)
- PC with serial terminal (PuTTY, Tera Term, Arduino Serial Monitor, etc.)

---

## Communication Settings
- Baud Rate: **115200**
- Data Bits: **8**
- Stop Bits: **1**
- Parity: **None**
- Flow Control: **None**

---

## How It Works

1. MCU initializes:
   - HAL library
   - System clock
   - GPIO for LED
   - USART2 for UART communication

2. Program continuously runs in an infinite loop:
   - Sends prompt:
     ```
     Type 1=LED ON, 0=LED OFF:
     ```
   - Waits for a single byte input via UART

3. Based on received input:
   - `'1'` → LED turns ON (GPIO reset) and sends `"LED ON"`
   - `'0'` → LED turns OFF (GPIO set) and sends `"LED OFF"`
   - Any other input → `"Unknown!"`

---

## Code Logic Summary
- Uses `HAL_UART_Receive()` for blocking input
- Uses `HAL_UART_Transmit()` for responses
- Uses GPIO write functions to control LED state
- Simple conditional logic for command handling

---

## GPIO Configuration
- `LEDOUT_Pin` is configured as:
  - Output Push-Pull
  - No Pull-up / Pull-down
  - Low speed

---

## Example Output (Serial Monitor)
Type 1=LED ON, 0=LED OFF:    
1    
LED ON

Type 1=LED ON, 0=LED OFF:    
0    
LED OFF   


---

## Notes
- Newline character (`\n`) is ignored and re-prompts input
- Blocking UART receive is used (`HAL_MAX_DELAY`)
- LED logic assumes active-low configuration (RESET = ON)

---