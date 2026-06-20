# UART-Controlled PWM Generator v2 — STM32F103 (TIM1)
### Variable-Length Command Parsing

## Overview

Generates a PWM signal on **TIM1 Channel 1**, with frequency and duty cycle set live over UART. This version accepts a **variable-length** command (unlike the fixed 12-byte v1) terminated by `\n`, parsed at runtime.

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

Send over UART, terminated with **Enter (`\n`)**:

```
$freq:duty
```

| Field | Meaning |
|-------|---------|
| `$` | Start marker (required first character) |
| `freq` | Frequency in Hz, any number of digits |
| `:` | Separator |
| `duty` | Duty cycle %, any number of digits |

**Example:**
```
$1000:50
```
→ Frequency = 1000 Hz, Duty = 50%

### Rules
- Frequency = `0` or > `50000` → rejected
- Duty > `100` → rejected
- Must start with `$`, otherwise the whole line is discarded as an error

---

## How It Works

### 1. Reception — `HAL_UART_RxCpltCallback`

```
Byte received
   │
   ├── First byte is '$'  →  start capturing into u2Data[]
   │      └── on '\n'  →  strip '\n', replace u2Data[0] '$' with '@',
   │                       set pwm_flag = 1 (ready to parse)
   │
   └── '\n' received without a valid '$' start  →  pwm_flag = 2 (error)
```

The buffer is rewritten with `@` as a marker so the main loop's parser knows where the frequency field begins.

### 2. Parsing — Main Loop

```
u2Data: "@1000:50\0"
          │   │  │
          │   │  └─ duty digits, read until '\0'
          │   └──── ':' separator (parsing stops here for freq)
          └──────── '@' marker, parsing starts at index 1

P_PWM.U_Freq = digits before ':'   (accumulated: freq = freq*10 + digit)
P_PWM.U_Duty = digits after ':'    (accumulated: duty = duty*10 + digit)
```

### 3. Validation & PWM Update

```c
if freq == 0 or freq > 50000  →  error, request resend
if duty > 100                  →  error, request resend

R_Freq = 1,000,000 / freq        // ARR value
R_Duty = (duty * R_Freq) / 100   // CCR1 value

TIM1->ARR  = R_Freq - 1
TIM1->CCR1 = R_Duty
```

---

## Build & Flash

1. Open in **Keil MDK**, build, flash via **ST-Link**.
2. Open serial terminal at **115200 baud, 8N1**.
3. On boot, device prints usage instructions.
4. Send `$1000:50` + Enter → sets 1000 Hz, 50% duty.

---

## ⚠️ Code Status — Incomplete / In Progress

This file is **not a finished, production-ready module**. A few things to be aware of before relying on it:

- The fixed `u2Data[12]` buffer has **no bounds check** in the ISR's `else pwm_count++;` path — a line longer than 12 characters before `$...\n` will overflow the array. Add a `pwm_count < sizeof(u2Data)-1` guard before writing.
- `count0` is a **global** used only inside the main-loop parser; if `HAL_UART_RxCpltCallback` fires again mid-parse (unlikely here since parsing is fast, but worth noting) there's no protection against re-entrancy.
- The `'@'` marker reuse of `u2Data[0]` (overwriting `'$'`) is a workaround rather than a clean parser — fine for prototyping, but worth replacing with a proper tokenizer if this evolves further.
- Min frequency is still bounded by the 16-bit `ARR` register (~15 Hz floor) even though the duty/freq validation only checks the upper bound (50000 Hz) and zero — very low frequencies between 1–14 Hz will silently produce incorrect timing rather than being rejected.

**Treat this as a working prototype, not a finished driver** — functionally it runs and responds to commands, but the issues above should be addressed before calling it complete.