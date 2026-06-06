# STM32 UART DMA Transmission Example (HAL)

## 📌 Overview
This project demonstrates UART transmission using **DMA (Direct Memory Access)** on an STM32 microcontroller (HAL library). The program continuously sends a 250-byte buffer over **USART2** while using DMA callbacks to modify the buffer dynamically during transmission. An onboard LED is also toggled in the main loop.

---

## ⚙️ Features
- UART2 transmission at **115200 baud rate**
- DMA-based non-blocking data transmission
- Half-transfer and full-transfer DMA callbacks
- Dynamic buffer modification during transmission
- LED blink for visual system activity indication
- Automatic DMA stop after a condition is met

---

## 🧠 How It Works

### 1. Initialization
- System clock configured using HSI
- GPIO initialized for LED output
- USART2 initialized for TX/RX
- DMA configured for USART2 TX

---

### 2. Transmission Buffer
A 250-byte array is used:

```c
uint8_t TxData[250];

for(uint8_t i = 0; i < 250; i++)
{
    TxData[i] = i;
}
```
### 3. DMA Transmission

- DMA transmission is started once in 
```C
main():

HAL_UART_Transmit_DMA(&huart2, TxData, 250);
```

- This sends data over UART without blocking CPU execution.

### 4. DMA Callbacks
Half Transfer Callback

- Triggered when first half (0–124) is transmitted:
```c
void HAL_UART_TxHalfCpltCallback(UART_HandleTypeDef *huart)
{
    for(uint8_t i = 0; i < 125; i++)
    {
        TxData[i] = temp;
    }
    temp++;
}
```
- Full Transfer Callback

- Triggered when full buffer is transmitted:
```c
void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart)
{
    for(uint8_t i = 125; i < 250; i++)
    {
        TxData[i] = temp;
    }
    temp++;

    if(temp > 59)
        HAL_UART_DMAStop(&huart2);

    if (huart->Instance == USART2)
        flag0 = 1;
}
```

### 5. LED Blinking Loop

- Main loop toggles LED every 500 ms:
```c
HAL_GPIO_WritePin(LEDOUT_GPIO_Port, LEDOUT_Pin, GPIO_PIN_RESET);
HAL_Delay(500);
HAL_GPIO_WritePin(LEDOUT_GPIO_Port, LEDOUT_Pin, GPIO_PIN_SET);
HAL_Delay(500);
```

### Program Behavior
    - UART continuously transmits 250-byte blocks via DMA
    - Buffer content changes after each half/full transfer
    - Transmission stops when temp > 59
    - LED blinks continuously as a heartbeat indicator