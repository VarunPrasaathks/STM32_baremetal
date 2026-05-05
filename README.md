# STM32 Baremetal Learning Repository

This repository contains my hands-on learning journey with STM32 microcontrollers using bare-metal programming (no HAL/LL abstraction).

The goal of this repo is to build a strong understanding of embedded systems by working directly with registers and implementing core peripherals from scratch.

## 🚦 Start Here

Begin with: `led_blinkmyown`

Then follow this exact order:

1. `led_blinkmyown`
2. `debounce_bm`
3. `debounce_toggle_bm`
4. `interrupt_bm`
5. `singleLEDblink`
6. `timer_basicledblink_bm`
7. `timer_using_interrupt_bm`
8. `pwm_basic_bm`
9. `pwm_fading_bm`
10. `pwm_multichannel_bm`
11. `onepulsemode_timer_bm`
12. `input_capture_basic_bm`
13. `togglemode_output_capture_bm`
14. `gatedmode_slavetimer_bm`
15. `adc_potentiometer_pwm_bm`
16. `adc_interrupt_potentiometer_pwm`
17. `ADC_blink_project_bm`
18. `UART_basic_bm`
19. `UART_rx_project_bm`
20. `UART_interrupt_bm`
21. `UART_ring_buffer_bm`
22. `UART_Framing_FSM_bm`
23. `UART_FSM_payload_basic_bm`
24. `UART_Framing_FSM_ringbuffer_bm`
25. `UART_Framing_FSM_Checksum_basic_bm`
26. `UART_command_frames_basic_bm`
27. `UART_command_frames_bm`
28. `UART_ack_basic`
29. `UART_test_logic_analyzer_bm`
30. `SPI_basic_bm`
31. `spi_interrupt_basic_bm`

---

## 📚 Topics Covered (Learning Path)

The projects are structured in a progressive manner. Follow this order for best understanding:

### 🔰 Recommended Learning Order

1. **LED Blink (Start Here)**

2. **Debounce**

3. **Timers**

4. **ADC (Analog to Digital Conversion)**

5. **UART (Communication Protocols)**

6. **(Upcoming) SPI / I2C**

Each topic contains multiple projects exploring concepts in depth.

---

## 📂 Repository Structure

Each folder represents a standalone project:

* `*_bm` → Bare-metal implementation
* Each project includes:

  * Fully working code
  * Detailed inline documentation
  * Focus on understanding registers and peripherals

Example:

* `UART_basic_bm` → Basic UART transmission
* `UART_ring_buffer_bm` → Efficient UART using ring buffer
* `ADC_blink_project_bm` → ADC-based control

---

## 🚀 Key Highlights

* No HAL or libraries – pure register-level programming
* Focus on understanding **how hardware actually works**
* Covers advanced concepts like:

  * Finite State Machines (FSM)
  * Ring Buffers
  * Interrupt-driven design

---

## 🛠️ Tools & Environment

* STM32 Microcontroller (e.g., STM32F4 series)
* ST-Link
* ARM GCC Toolchain
* STM32CubeIDE / Makefile-based build
* Logic Analyzer

---

## 📖 How to Use This Repo

1. Clone the repository
   git clone [https://github.com/VarunPrasaathks/STM32_baremetal.git](https://github.com/VarunPrasaathks/STM32_baremetal.git)

2. Open any project folder

3. Read the code (important!)

   * Most explanations are written inside the code itself

4. Build and flash using your preferred toolchain

---

## 🧠 Learning Approach

Instead of just making projects, I focused on:

* Understanding registers and datasheets
* Writing modular and readable code
* Exploring multiple implementations of the same concept

---

## 🔄 Ongoing Work

This repository is actively being updated as I continue learning.

Upcoming additions may include:

* SPI / I2C communication
* DMA
* RTOS (FreeRTOS integration)
* Advanced driver development

---

## 🤝 Contributions

Suggestions, improvements, and discussions are welcome!

---

## ⭐ If this helped you

Consider giving a star ⭐ to support the work!
