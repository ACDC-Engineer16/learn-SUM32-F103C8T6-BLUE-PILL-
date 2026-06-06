# STM32 PWM Control via UART (TIM1 + USART2)

## 📌 Overview
This project demonstrates **PWM control using STM32 TIM1**, where the duty cycle is dynamically updated via **UART (USART2)** communication.

You can send data from a PC terminal (like PuTTY / TeraTerm / Serial Monitor) to control LED brightness or PWM output.

---

## ⚙️ Features
- PWM generation using TIM1 Channel 1
- Real-time duty cycle control via UART
- Interrupt-based UART reception (non-blocking)
- Simple serial protocol (2-digit value + newline)
- GPIO support for optional LED output

---

## 🔌 Hardware Requirements
- STM32 Microcontroller (with TIM1 + USART2)
- LED + resistor (220Ω / 330Ω)
- USB-to-UART (if not using ST-Link VCP)
- ST-Link programmer

---

## 📍 Pin Configuration
| Signal | Pin |
|--------|-----|
| TIM1_CH1 PWM Output | PA8 (typical STM32F1) |
| USART2 TX | PA2 |
| USART2 RX | PA3 |
| GPIO LED (optional) | LEDOUT_Pin (board specific) |

> ⚠️ Pin mapping depends on your STM32 board and CubeMX setup.

---

## 🧠 Working Principle

### 1. PWM Setup
- Timer: TIM1
- Mode: PWM Mode 1
- Prescaler: 7
- Period: 99  
→ PWM range: **0–100 duty cycle**

---

### 2. UART Control Logic
The MCU receives **3 bytes**:
