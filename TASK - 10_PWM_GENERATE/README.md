# STM32 PWM Duty Cycle Fade (TIM1 + HAL)

## 📌 Overview
This project demonstrates **PWM signal generation using TIM1** on an STM32 microcontroller. The duty cycle is smoothly increased and decreased in a loop to create a **fade-in / fade-out effect**, commonly used for LED brightness control.

---

## ⚙️ Features
- PWM generation using **TIM1 Channel 1**
- Duty cycle modulation (0% → 100% → 0%)
- Smooth LED fading effect
- Hardware timer-based PWM (no blocking PWM generation)
- Simple GPIO-based LED output

---

## 🧠 How It Works

### 1. Timer Configuration
TIM1 is configured for PWM generation:

- Prescaler: `8-1`
- Period: `100-1`
- PWM Mode: `PWM1`
- Pulse (initial duty): `0`

This results in a PWM resolution of **0–100 steps (percentage-like control)**.

---

### 2. PWM Start

PWM is started on TIM1 Channel 1:

```c
HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_1);
```
 ---

### 3. Duty Cycle Control Logic

- A variable DUTY is used to smoothly increase and decrease brightness.
```C
uint8_t DUTY = 0;
uint8_t i = 1;
```

  ---

### 4. Main Loop Behavior

- The duty cycle increases from 0 → 100, then decreases back to 0 repeatedly:
```C
if(i){
    DUTY++;
    if(DUTY == 100)
        i = 0;
}
else{
    DUTY--;
    if(!DUTY)
        i = 1;
}

TIM1->CCR1 = DUTY;
HAL_Delay(50);
```
--- 

### Result
- LED brightness smoothly fades up and down
- Continuous PWM modulation using hardware timer
- No CPU-heavy toggling required

### Key Variables
|Variable|	Purpose|
|--------|----------|
|DUTY	|Controls PWM duty cycle|
|i	|Direction flag (up/down counting)|
|TIM1->CCR1|	PWM compare register|


### Working Principle
- Timer generates constant PWM signal
- CCR1 changes duty cycle dynamically
- LED brightness changes accordingly
- Loop creates smooth fading effect
