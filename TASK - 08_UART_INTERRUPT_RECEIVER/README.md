# STM32 UART Interrupt Receive (Echo) with LED Blink

## Overview

This project demonstrates **UART reception using interrupts** on the STM32F103C8T6 (Blue Pill). It receives characters one at a time over USART2 via `HAL_UART_Receive_IT()`, accumulates them into a buffer until a newline (`\n`) is detected or the buffer is full, then **echoes the complete string back** over UART. The onboard LED blinks independently in the main loop.

---

## Hardware

| Component | Detail |
|-----------|--------|
| MCU | STM32F103C8T6 (Blue Pill) |
| LED Pin | PC13 (LEDOUT) — Active LOW |
| UART | USART2 (PA2 TX, PA3 RX) |
| Adapter | USB-to-TTL (CH340 / CP2102) on PA2/PA3 |

---

## Peripheral Configuration

### System Clock
- Source: **HSI (8 MHz internal oscillator)**
- PLL: Disabled
- All bus dividers: ÷1

### USART2
| Parameter | Value |
|-----------|-------|
| Baud Rate | 115200 |
| Word Length | 8 bits |
| Stop Bits | 1 |
| Parity | None |
| Mode | TX + RX |
| Hardware Flow Control | None |
| Oversampling | 16x |

### GPIO (PC13)
- Mode: Output Push-Pull
- Speed: Low
- Pull: None

---

## How It Works

### Initialization
- `HAL_UART_Receive_IT(&huart2, &temp, 1)` is called once before the main loop.
- This arms the UART interrupt to fire every time **1 byte** is received.

### Receive Interrupt Callback — `HAL_UART_RxCpltCallback`

Called automatically on each received byte:

1. Stores the received byte (`temp`) into `RxData[index]`.
2. Increments `index`.
3. Re-arms the interrupt for the next byte: `HAL_UART_Receive_IT(&huart2, &temp, 1)`.
4. Checks for end-of-string conditions:
   - If `temp == '\n'` **or** `index == 20` (buffer full):
     - Null-terminates `RxData`.
     - Resets `index = 0`.
     - Sets `flag0 = 1` to notify the main loop.

### Main Loop

```
if (flag0 == 1):
    flag0 = 0
    find length of RxData (scan until null terminator)
    HAL_UART_Transmit() → echo RxData back over UART

LED ON → 500 ms → LED OFF → 500 ms → repeat
```

> **Note:** The LED blink introduces a ~1 second delay per loop cycle. Characters received during `HAL_Delay()` are safely captured by the interrupt and stored in the buffer. However, if a complete message arrives and `flag0` is set again before the main loop checks it, the previous message will be overwritten. For high-throughput use, consider removing the LED delay or using a ring buffer.

---

## Key Variables

| Variable | Type | Description |
|----------|------|-------------|
| `RxData[20]` | `uint8_t[]` | Accumulation buffer for received characters |
| `temp` | `uint8_t` | Single-byte staging variable for IT reception |
| `flag0` | `uint8_t` | Set when a complete string is ready to echo |
| `index` | `uint8_t` | Write position in `RxData[]` |

---

## Usage

1. Connect USB-to-TTL adapter: **PA2 (TX) → RXD**, **PA3 (RX) → TXD**, **GND → GND**.
2. Open a serial terminal (e.g., PuTTY, Tera Term) at **115200 baud, 8N1**.
3. Type a message and press **Enter** (`\n`).
4. The Blue Pill echoes the message back within ~1 second.

---

## Files

| File | Description |
|------|-------------|
| `main.c` | Application entry point, interrupt callback, echo logic |
| `main.h` | Pin definitions (`LEDOUT_Pin`, `LEDOUT_GPIO_Port`) |

---

## Build & Flash

1. Open the project in **Keil MDK V5**.
2. Build with **Ctrl+F7**.
3. Flash via **ST-Link V2** (ensure STSW-LINK009 driver is installed).
4. Open terminal at **115200 baud, 8N1** on the USB-to-TTL COM port.

---

## Notes

- Buffer size is **20 bytes** (including the null terminator). Messages longer than 19 characters will be cut off and echoed immediately.
- The `\n` character itself is stored in the buffer before the null terminator, so the echo will include it.
- PC13 is **active LOW**: `GPIO_PIN_RESET` = LED ON, `GPIO_PIN_SET` = LED OFF.