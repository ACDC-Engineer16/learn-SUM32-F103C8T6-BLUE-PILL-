# Button Interrupt with Relay Control

## Objective

Control two relays using a push button interrupt with software debouncing.

## Hardware

* STM32 Blue Pill (STM32F103C8T6)
* Push Button
* 2-Channel Relay Module
* Onboard LED (PC13)

## Configuration

* PC13 → LED Output
* SWITCH_Pin → External Interrupt (Rising/Falling Edge)
* RELAY1_Pin → Output
* RELAY2_Pin → Output

## Features

* External Interrupt (EXTI)
* Software Debouncing (50 ms)
* Relay Control
* Non-Blocking LED Blink (500 ms)

## Main Logic

```c
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
    if(GPIO_Pin == SWITCH_Pin)
    {
        btnEdgeDetected = 1;
        btnEdgeTime = HAL_GetTick();
    }
}
```

```c
if(btnEdgeDetected)
{
    if((HAL_GetTick() - btnEdgeTime) >= 50)
    {
        btnEdgeDetected = 0;

        if(HAL_GPIO_ReadPin(SWITCH_GPIO_Port, SWITCH_Pin) == GPIO_PIN_RESET)
        {
            HAL_GPIO_WritePin(RELAY1_GPIO_Port, RELAY1_Pin, GPIO_PIN_SET);
            HAL_GPIO_WritePin(RELAY2_GPIO_Port, RELAY2_Pin, GPIO_PIN_SET);
        }
    }
}
```

## Working

* Button edge generates an interrupt.
* Interrupt only records the event and timestamp.
* Main loop waits 50 ms for debounce.
* Relays are switched according to button state.
* PC13 LED toggles every 500 ms without blocking the program.

## Learning

* GPIO External Interrupt (EXTI)
* Interrupt Callback Function
* Software Debouncing
* Non-Blocking Timing using HAL_GetTick()
* Relay Control using GPIO
