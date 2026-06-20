# UART-Controlled PWM Generator — STM32F103 (TIM1)

## Overview

Generates a PWM signal on **TIM1 Channel 1**, with **frequency and duty cycle set live over UART** using a simple custom command string.

---

## Hardware

| Item | Detail |
|------|--------|
| MCU | STM32F103C8T6 (Blue Pill) |
| Timer | TIM1, Channel 1 (PWM output) |
| UART | USART2, 115200 baud, 8N1 |
| Clock | HSI 8 MHz, Timer prescaler = 8 → 1 MHz timer clock |

---

## Command Format

Send over UART (followed by Enter):

```
$xxx:x:xxx$
```

| Field | Meaning |
|-------|---------|
| `xxx` (1st) | Frequency value (0–999) |
| `x` (2nd) | Unit: `1` = KHz, `0` = Hz |
| `xxx` (3rd) | Duty cycle % (0–100) |

**Example:**
```
$010:1:050$
```
→ Frequency = 10 KHz, Duty = 50%

### Rules
- Frequency = `0` → rejected
- Duty > `100` → rejected
- Command must be exactly 12 bytes: `$` + 3 digits + `:` + 1 digit + `:` + 3 digits + `$` + `\n`

---

## How It Works

1. UART receives 1 byte at a time via interrupt (`HAL_UART_Receive_IT`).
2. Bytes are collected into `u2Data[12]`.
3. On the 12th byte, the frame is validated (`$`, `:`, `:`, `$`, `\n` in correct positions).
4. If valid → parses frequency, unit, and duty → sets `pwm_flag = 1`.
5. If invalid or malformed → sets `pwm_flag = 2` (error).
6. Main loop checks `pwm_flag`:
   - `1` → calculates new `ARR` (period) and `CCR1` (duty) → updates `TIM1` registers.
   - `2` → prints error message, reprints instructions.

### PWM Calculation

```
Timer clock = 1 MHz (PWM_MAX_FREQ)

R_Freq = Timer clock / (frequency × (1000 if KHz else 1))   → becomes ARR
R_Duty = (Duty% × R_Freq) / 100                               → becomes CCR1

TIM1->ARR  = R_Freq - 1
TIM1->CCR1 = R_Duty
```

**Range:** ~15 Hz minimum (1 MHz / 65535) up to ~50 KHz (with usable duty resolution).

---

## Build & Flash

1. Open in **Keil MDK**, build, flash via **ST-Link**.
2. Open serial terminal at **115200 baud, 8N1**.
3. On boot, the device prints usage instructions automatically.
4. Send a command like `$025:0:075$` to set 25 Hz, 75% duty.
5. PWM output appears on the TIM1 CH1 pin (per `HAL_TIM_MspPostInit` GPIO mapping).

---

## Notes

- Only `pwm_count == 12` triggers parsing — shorter/longer inputs are ignored or reset on `\n`.
- Sending `\n` early resets the buffer and flags an error.
- Echoes the received command back over UART for confirmation.