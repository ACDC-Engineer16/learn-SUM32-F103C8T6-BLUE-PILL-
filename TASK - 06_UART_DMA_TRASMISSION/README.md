# UART DMA Transmission

## Objective

Transmit 250 bytes using UART DMA mode while blinking the onboard LED.

## Hardware

* STM32 Blue Pill (STM32F103C8T6)
* USB-to-TTL Converter
* Onboard LED (PC13)

## Configuration

* UART: USART2
* Baud Rate: 115200
* DMA: UART TX (DMA1 Channel 7)
* LED Pin: PC13

## Main Logic

```c
flag0 = 1;
while (1)
  {
	if(flag0 == 1){
		HAL_UART_Transmit_DMA(&huart2,TxData,250);
		flag0 = 0;
	}
	// other code work
  }
```

```c
void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart)
{
    if(huart->Instance == USART2)
        flag0 = 1;
}
```

## Working

* A buffer containing values 0–249 is created.
* DMA transfers the data buffer to USART2 automatically.
* CPU is free while DMA handles the transmission.
* After transmission completes, a callback function sets a flag for the next transfer.
* The onboard LED blinks every 500 ms.

## Learning

* UART DMA Transmission
* DMA Configuration
* UART Transfer Complete Callback
* CPU Offloading using DMA. **So that more faster then IT call**
* Concurrent UART and GPIO Operations
