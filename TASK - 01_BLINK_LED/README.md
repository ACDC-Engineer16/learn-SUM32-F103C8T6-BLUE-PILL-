# LED Blink - STM32 Blue Pill

## Objective

Blink the onboard LED connected to PC13 using STM32 HAL functions.

## Hardware

* STM32 Blue Pill (STM32F103C8T6)
* Onboard LED (PC13)

## Configuration

* GPIO Pin: PC13
* Mode: Output Push-Pull
* Clock Source: HSI (Internal Oscillator)

## Code

```c
while (1)
{
    HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PIN_RESET); // LED ON
    HAL_Delay(500);

    HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PIN_SET);   // LED OFF
    HAL_Delay(500);
}
```

## Working

The onboard LED connected to PC13 turns ON for 500 ms and OFF for 500 ms repeatedly.

> Note: On the Blue Pill board, the PC13 LED is typically **active LOW**, meaning `GPIO_PIN_RESET` turns the LED ON and `GPIO_PIN_SET` turns it OFF.

## Output

LED blinks continuously with a 1-second cycle.

## Learning

* GPIO Output Configuration
* STM32 HAL GPIO Functions
* LED Control using PC13
