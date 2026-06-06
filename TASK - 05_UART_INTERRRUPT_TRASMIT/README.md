# UART Interrupt Transmission

## Objective

Transmit 250 bytes continuously using UART Interrupt mode while blinking the onboard LED.

## Hardware

* STM32 Blue Pill (STM32F103C8T6)
* USB-to-TTL Converter
* Onboard LED (PC13)

## Configuration

* UART: USART2
* Baud Rate: 115200
* Transmission Mode: Interrupt (`HAL_UART_Transmit_IT`)
* LED Pin: PC13

## Main Logic

```c
HAL_UART_Transmit_IT(&huart2, TxData, 250);
```

```c
void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart)
{
    if(huart->Instance == USART2)
        flag0 = 1;
}
```

## Working

* A buffer containing 250 bytes (0–249) is created.
* Data is transmitted using UART Interrupt mode.
* After transmission completes, the callback function is executed.
* The callback sets a flag to start the next transmission.
* The onboard LED on PC13 blinks every 500 ms.

## Learning

* UART Interrupt Transmission
* Callback Functions
* Non-Blocking Communication
* UART Data Buffer Handling
* LED Control using GPIO
