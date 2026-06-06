# Relay Control Using Push Button

## Objective

Control two relays using a push button input.

## Hardware

* STM32 Blue Pill (STM32F103C8T6)
* Push Button
* 2-Channel Relay Module

## GPIO Configuration

* SWITCH_Pin → Input
* RELAY1_Pin → Output
* RELAY2_Pin → Output

## Code

```c
if(HAL_GPIO_ReadPin(SWITCH_GPIO_Port, SWITCH_Pin))
{
    HAL_GPIO_WritePin(RELAY1_GPIO_Port, RELAY1_Pin, 0);
    HAL_GPIO_WritePin(RELAY2_GPIO_Port, RELAY2_Pin, 0);
}
else
{
    HAL_Delay(50);

    if(HAL_GPIO_ReadPin(SWITCH_GPIO_Port, SWITCH_Pin) == 0)
    {
        HAL_GPIO_WritePin(RELAY1_GPIO_Port, RELAY1_Pin, 1);
        HAL_GPIO_WritePin(RELAY2_GPIO_Port, RELAY2_Pin, 1);
    }
}
```

## Working

* Button Pressed → Relay1 and Relay2 ON.
* Button Released → Relay1 and Relay2 OFF.
* 50 ms delay is used for simple button debouncing.

## Learning

* GPIO Input Reading
* GPIO Output Control
* Push Button Interface
* Relay Control
* Basic Debouncing using HAL_Delay()
