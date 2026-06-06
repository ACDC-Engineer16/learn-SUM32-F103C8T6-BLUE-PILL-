# STM32 UART DMA Receive (Large Buffer) with LED Toggle

## Overview

This project demonstrates **UART reception of large data using DMA with double-buffering** on the STM32F103C8T6 (Blue Pill). It receives a **size-prefixed data stream** over USART2 into a 256-byte circular DMA buffer, then assembles the complete payload into a 4096-byte `FinalBuf[]`. The DMA half-transfer and full-transfer callbacks coordinate with the main loop to handle data of arbitrary size. The onboard LED toggles every second to confirm the system is alive.

---

## Hardware

| Component | Detail |
|-----------|--------|
| MCU | STM32F103C8T6 (Blue Pill) |
| LED Pin | PC13 (LEDOUT) — Active LOW |
| UART | USART2 (PA2 TX, PA3 RX) |
| DMA | DMA1 Channel 6 (USART2 RX) |
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

### DMA (USART2 RX)
- Channel: **DMA1 Channel 6**
- IRQ Priority: 0 (highest)
- Interrupt: Enabled
- Buffer Size: 256 bytes (split into two 128-byte halves)

### GPIO (PC13)
- Mode: Output Push-Pull
- Speed: Low
- Pull: None

---

## Protocol

The sender must prefix the data with a **4-digit ASCII size field**:

```
[S3][S2][S1][S0][DATA...]
```

| Bytes | Meaning |
|-------|---------|
| `[0]` | Thousands digit (ASCII) |
| `[1]` | Hundreds digit (ASCII) |
| `[2]` | Tens digit (ASCII) |
| `[3]` | Units digit (ASCII) |
| `[4..N]` | Actual payload data |

**Example:** To send 500 bytes of data, prefix with `"0500"` followed by the 500 data bytes.

> Maximum payload size: **4092 bytes** (4096 buffer minus 4 header bytes).

---

## How It Works

### DMA Setup
`HAL_UART_Receive_DMA(&huart2, RxData, 256)` sets up a **256-byte circular DMA buffer**. The DMA fires two interrupts:
- **Half Transfer** → after 128 bytes received (`RxData[0..127]` is ready)
- **Full Transfer** → after 256 bytes received (`RxData[128..255]` is ready)

### Half Transfer Callback — `HAL_UART_RxHalfCpltCallback`

**First call (size not yet received):**
1. Parses the 4-byte ASCII size prefix from `RxData[0..3]`.
2. Copies `RxData[4..127]` (124 bytes of payload) into `FinalBuf`.
3. Clears the first 128 bytes of `RxData`.
4. Advances `indx` by 128, sets `isSizeRxed = 1`, `HTC = 1`.

**Subsequent calls:**
1. Copies `RxData[0..127]` into `FinalBuf + indx`.
2. Clears the first 128 bytes of `RxData`.
3. Advances `indx` by 128, sets `HTC = 1`.

### Full Transfer Callback — `HAL_UART_RxCpltCallback`
1. Copies `RxData[128..255]` into `FinalBuf + indx`.
2. Clears the second half of `RxData`.
3. Advances `indx` by 128, sets `HTC = 0` (clears half-transfer flag; full transfer is implicitly tracked by `FTC`).

### Main Loop — Termination Logic

The main loop checks whether the **remaining expected bytes fit within one DMA half-buffer** (i.e., the transfer is nearly complete):

```
if ((size - indx) > 0 AND (size - indx) < 128):
    if HTC == 1:  copy from RxData[128..], reset state, restart DMA
    if FTC == 1:  copy from RxData[0..],   reset state, restart DMA

else if indx == size AND (HTC or FTC):
    reset state, restart DMA
```

After each check, the DMA is stopped and restarted to prepare for the next message. The LED toggles every 1 second.

---

## Key Variables

| Variable | Type | Description |
|----------|------|-------------|
| `RxData[256]` | `uint8_t[]` | Circular DMA receive buffer (two 128-byte halves) |
| `FinalBuf[4096]` | `uint8_t[]` | Assembled complete payload |
| `size` | `uint32_t` | Total expected payload size (parsed from header) |
| `indx` | `uint32_t` | Current write position in `FinalBuf` |
| `isSizeRxed` | `int` | Flag: size header has been parsed |
| `HTC` | `int` | Half Transfer Complete flag |
| `FTC` | `int` | Full Transfer Complete flag (set in main loop path) |

---

## Files

| File | Description |
|------|-------------|
| `main.c` | Application entry, DMA callbacks, assembly logic |
| `main.h` | Pin definitions (`LEDOUT_Pin`, `LEDOUT_GPIO_Port`) |

---

## Build & Flash

1. Open the project in **Keil MDK V5**.
2. Build with **Ctrl+F7**.
3. Flash via **ST-Link V2** (ensure STSW-LINK009 driver is installed).
4. Send data from a PC terminal or script at **115200 baud, 8N1** with the 4-byte size prefix.

---

## Notes & Limitations

- **`FTC` is never explicitly set to `1`** in the current callbacks — the `FTC == 1` branch in the main loop is unreachable without modification. If needed, set `FTC = 1` at the end of `HAL_UART_RxCpltCallback`.
- The `HAL_Delay(1000)` in the main loop limits polling rate to 1 Hz. For faster or higher-baud transfers, reduce or remove the delay and use a flag-based approach instead.
- `strcpy` is used for copying in the main loop — this is safe only for null-terminated strings. For binary data, replace with `memcpy` and explicit lengths.
- PC13 is **active LOW**: `GPIO_PIN_RESET` = LED ON, `GPIO_PIN_SET` = LED OFF. `HAL_GPIO_TogglePin` alternates between both states.