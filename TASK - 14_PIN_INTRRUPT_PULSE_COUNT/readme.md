# EXTI Pulse Counter with UART Output — STM32F103

## Overview

Counts rising-edge pulses on an external interrupt pin and reports the live count over UART whenever it changes. Also toggles the onboard LED on every pulse.

---

## Hardware

| Item | Detail |
|------|--------|
| MCU | STM32F103C8T6 (Blue Pill) |
| Input | `E_COUNTER_Pin` — EXTI, rising edge trigger |
| Output | PC13 (onboard LED) — toggled per pulse |
| UART | USART2, 115200 baud, 8N1 |
| Clock | HSI 8 MHz |

---

## How It Works

### Pulse Detection — `HAL_GPIO_EXTI_Callback`

```c
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
    if(GPIO_Pin == E_COUNTER_Pin)
    {
        counter++;
        HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_13);   // LED blinks per pulse
    }
}
```

Every rising edge on `E_COUNTER_Pin`:
1. Increments the global `counter`.
2. Toggles PC13 — visual confirmation of each pulse.

### Reporting — Main Loop

```c
if(last_count != counter)
{
    last_count = counter;
    sprintf(buf, "Count: %lu\r\n", (unsigned long) last_count);
    HAL_UART_Transmit(&huart2, (uint8_t*)buf, strlen(buf), 100);
}
```

The main loop only transmits when the count has **changed since last check** — avoids flooding UART with repeated identical values during idle periods.

---

## Example Output

```
Count: 1
Count: 2
Count: 3
...
```

One line per new pulse, printed as soon as the main loop notices the change.

---

## Build & Flash

1. Open in **Keil MDK**, build, flash via **ST-Link**.
2. Connect pulse source (button, sensor, encoder, etc.) to `E_COUNTER_Pin`.
3. Open serial terminal at **115200 baud, 8N1**.
4. Trigger pulses — count increments and prints, LED blinks each time.

---

## Notes

- `counter` is `volatile` — correctly shared between the EXTI ISR and the main loop.
- No debounce — mechanical switches will cause multiple counts per press unless externally debounced (hardware RC filter or Schmitt trigger).
- `counter` is `uint32_t`; wraps around after ~4.29 billion pulses (not a practical concern).
- GPIOB clock is enabled (`__HAL_RCC_GPIOB_CLK_ENABLE`) but unused directly in this file — likely reserved by `E_COUNTER_Pin` if it's on GPIOB.