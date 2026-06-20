# PWM Burst Generator — Fixed Pulse Count Stop (TIM1 + TIM2)
### STM32F103 | 72 MHz PLL (HSE 8MHz × 9)

## Overview

Generates a PWM signal on **TIM1 Channel 1** and **automatically stops it after exactly 1000 PWM cycles**, using **TIM2 as a slave counter** clocked directly from TIM1's output pin — no software counting, no interrupts needed for counting itself.

---

## Hardware

| Item | Detail |
|------|--------|
| MCU | STM32F103 |
| Clock | HSE 8 MHz → PLL ×9 → 72 MHz SYSCLK |
| TIM1 | PWM generator, Channel 1 |
| TIM2 | Pulse counter (slave mode, external clock from TIM1 CH1 pin) |

---

## How It Works

### TIM1 — PWM Generator

```c
htim1.Init.Period = pwm_freq;      // 7199  → ARR
sConfigOC.Pulse    = pwm_duty;     // 3599  → CCR1 (~50% duty)
```

```
Timer clock = 72 MHz (no prescaler)
PWM frequency = 72,000,000 / (7199 + 1) = 10,000 Hz (10 kHz)
Duty cycle    = 3599 / 7200 ≈ 50%
```

### TIM2 — Pulse Counter (Slave Mode)

TIM2 does **not run on the internal clock** — it's configured as a **slave timer** that counts rising edges arriving on its trigger input, sourced from **TIM1's PWM output pin**:

```c
sSlaveConfig.SlaveMode      = TIM_SLAVEMODE_EXTERNAL1;
sSlaveConfig.InputTrigger   = TIM_TS_TI1FP1;          // input = TI1 (external pin)
sSlaveConfig.TriggerPolarity = TIM_TRIGGERPOLARITY_RISING;
htim2.Init.Period = pwm2_counter;   // 999  → counts 0..999 = 1000 pulses
```

```
TIM1 CH1 (PWM output) ──────→ TIM2 external clock input (TI1FP1)

Each PWM rising edge → TIM2 counter +1
TIM2 reaches ARR (999) → overflow → Update interrupt fires
```

### Auto-Stop on Target Reached

```c
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
    if(htim->Instance == TIM2)
    {
        HAL_TIM_PWM_Stop(&htim1, TIM_CHANNEL_1);   // stop PWM output
        HAL_TIM_Base_Stop_IT(&htim2);              // stop counting
        pwm_flag = 1;                              // signal "done"
    }
}
```

When TIM2 counts exactly **1000 pulses** (`TARGET_PULSES`), its update interrupt fires, which immediately stops TIM1's PWM output and TIM2 itself — producing a clean, hardware-timed burst of exactly 1000 cycles.

---

## Sequence Diagram

```
main()
 │
 ├── Reset TIM2 counter to 0
 ├── HAL_TIM_Base_Start_IT(&htim2)      ← arm pulse counter + interrupt
 ├── HAL_TIM_PWM_Start(&htim1, CH1)     ← PWM begins, TIM2 starts counting edges
 │
 │   [TIM1 generates 10 kHz PWM]
 │   [TIM2 counts each rising edge via TI1FP1]
 │
 │   ... after 1000 pulses ...
 │
 ├── TIM2 overflow → HAL_TIM_PeriodElapsedCallback()
 │        ├── PWM stopped (TIM1 CH1 off)
 │        ├── TIM2 stopped
 │        └── pwm_flag = 1
 │
 └── while(1) — idle (no further action coded)
```

---

## Build & Flash

1. Open in **Keil MDK**, build, flash via **ST-Link**.
2. Probe **TIM1 CH1 pin** with a scope/logic analyzer.
3. On power-up, PWM runs at 10 kHz, 50% duty, for exactly **1000 cycles (~100 ms)**, then stops automatically.
4. `pwm_flag` becomes `1` once complete — check via debugger or extend code to act on it.

---

## Notes

- This is a **hardware pulse-counting technique** — TIM2 requires no CPU time to count; it's entirely driven by the timer's slave-mode external clock input. Much more accurate than software counting in an ISR.
- `pwm_freq`, `pwm_duty`, and `pwm2_counter` are declared as `double` but used to initialize integer timer registers (`Period`, `Pulse`) — they get implicitly truncated to integers. Consider using `uint32_t` directly for clarity.
- `TARGET_PULSES` (1000) is defined but not actually used in the timer math — `pwm2_counter = 1000 - 1` is hardcoded separately. If you change `TARGET_PULSES`, also update `pwm2_counter` to match (or better, derive it: `pwm2_counter = TARGET_PULSES - 1`).
- The UART transmit line in `main()` is commented out — no USART is initialized in this file, so leave it commented unless USART2 init is added back.
- After the burst completes, the firmware does nothing further (`while(1)` is empty) — extend `HAL_TIM_PeriodElapsedCallback` or poll `pwm_flag` in the main loop to trigger a restart or next action.