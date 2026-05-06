# STM32 H7 Clock Computation Library 

This header-only library provides **compile-time** clock computation helpers for STM32H743 and STM32H723 microcontrollers, relying solely on CMSIS definitions. It aims to offer precise clock frequency calculations without runtime overhead, making it ideal for embedded systems where performance and determinism are critical.

## ✨ Features

*   **Compile-Time Evaluation:** Leverages C++ `constexpr` to perform all clock calculations during compilation, resulting in zero runtime overhead and optimized firmware.
*   **CMSIS-Based:** Directly uses standard CMSIS definitions (`stm32h7xx.h`) for maximum compatibility and adherence to microcontroller specifics.
*   **Comprehensive Coverage:** Calculates frequencies for:
    *   System Clock (SYSCLK)
    *   Core Clock (CPUCLK)
    *   High-Speed Clock (HCLK)
    *   APB Clocks (PCLK1, PCLK2, PCLK3, PCLK4)
    *   Timer Clocks (on APB1, APB2, APB4)
    *   PLL outputs (VCO, P, Q, R for PLL1, PLL2, PLL3)
    *   Peripheral Kernel Clocks (SPI, USART, I2C, RNG)
*   **Clear Snapshot:** Provides a `ClockSnapshot` struct to easily retrieve all computed clock frequencies at once.
*   **Robust Checks:** Includes basic guard checks and uses the latest `constexpr` features for cleaner code.
*   **MIT License:** Permissive license for broad usability.

## 📚 Understanding STM32 Clocks

Microcontrollers like the STM32H7 series have a complex system of internal and external clocks that drive various components. Understanding these is crucial for setting up peripherals correctly, achieving desired performance, and managing power consumption.

Here's a simplified breakdown:

1.  **Clock Sources:**
    *   **HSI (High-Speed Internal):** An internal RC oscillator, typically 16 MHz (but can vary and be divided). Fast to start, but less accurate than HSE.
    *   **CSI (Clock Security Internal):** Another internal RC oscillator, often used as a fallback or for specific modes.
    *   **HSE (High-Speed External):** An external crystal oscillator. More accurate and stable than internal oscillators, but requires external components.
    *   **LSE (Low-Speed External):** A 32.768 kHz crystal oscillator, typically used for the Real-Time Clock (RTC).
    *   **LSI (Low-Speed Internal):** An internal low-power RC oscillator, often used for the RTC when an external crystal isn't needed.

2.  **Clock Tree:**
    The STM32H7 features a sophisticated clock tree that routes and divides these sources to generate different clocks for the CPU, buses, and peripherals. Key stages include:
    *   **PLLs (Phase-Locked Loops):** Multipliers and dividers that can generate very high frequencies from basic sources (HSI, CSI, HSE). The H7 has multiple PLLs (PLL1, PLL2, PLL3), each with configurable inputs (M, N) and outputs (P, Q, R).
    *   **System Clock (SYSCLK):** The main clock for the microcontroller, selectable from HSI, CSI, HSE, or a PLL output (typically PLL1's P output).
    *   **CPU Clock (CPUCLK):** Derived from SYSCLK via the D1 Core Prescaler (`D1CPRE`). This clock drives the Cortex-M7 core.
    *   **AHB Bus Clocks (HCLK):** Derived from CPUCLK via the AHB Prescaler (`HPRE`). This clock drives high-speed peripherals and the memory interfaces.
    *   **APB Bus Clocks (PCLKx):** Derived from HCLK via APB Prescalers (`D1PPRE`, `D2PPRE1/2`, `D3PPRE`). These drive lower-speed peripherals. Peripherals connected to the same APB bus typically share the same clock.
    *   **Timer Clocks:** Often double the frequency of their parent APB bus clock (`PCLKx * 2`) if the APB prescaler is not 1.
    *   **Peripheral Kernel Clocks:** Specific clock sources selected for individual peripherals (e.g., SPI, USART, I2C) via dedicated multiplexers in the RCC (Reset and Clock Control) peripheral.

## 🚀 Getting Started

1.  **Include the header:** Add `#include "stm32h7_clocks_h743_h723.h"` to your C++ project.
2.  **Ensure CMSIS is present:** Make sure your build environment includes the correct CMSIS header (`stm32h7xx.h`) for your specific microcontroller.
3.  **Access clock values:** Use the `stm32::clocks` namespace and `constexpr` functions directly.

### Example Usage

```cpp
#include "stm32h7_clocks.h"
#include <iostream> // For demonstration purposes

// Assume RCC registers are properly configured beforehand!
// This library CALCULATES frequencies based on current RCC settings.

int main() {
    // --- Direct calculation ---
    uint32_t system_clock_freq = stm32::clocks::sysclk_hz();
    uint32_t hclk_freq = stm32::clocks::hclk_hz();
    uint32_t apb1_freq = stm32::clocks::pclk1_hz();
    uint32_t spi1_kernel_freq = stm32::clocks::spi123_kernel_hz(); // For SPI1, SPI2, SPI3

    std::cout << "System Clock: " << system_clock_freq << " Hz" << std::endl;
    std::cout << "HCLK: " << hclk_freq << " Hz" << std::endl;
    std::cout << "PCLK1: " << apb1_freq << " Hz" << std::endl;
    std::cout << "SPI1 Kernel Clock: " << spi1_kernel_freq << " Hz" << std::endl;

    // --- Using the snapshot ---
    stm32::clocks::ClockSnapshot clocks = stm32::clocks::snapshot();

    std::cout << "\n--- Clock Snapshot ---" << std::endl;
    std::cout << "SYSCLK: " << clocks.sysclk << " Hz" << std::endl;
    std::cout << "HCLK:   " << clocks.hclk << " Hz" << std::endl;
    std::cout << "PCLK1:  " << clocks.pclk1 << " Hz" << std::endl;
    std::cout << "PCLK2:  " << clocks.pclk2 << " Hz" << std::endl;
    std::cout << "PCLK3:  " << clocks.pclk3 << " Hz" << std::endl;
    std::cout << "PCLK4:  " << clocks.pclk4 << " Hz" << std::endl;
    std::cout << "PLL1 VCO: " << clocks.pll1_vco << " Hz" << std::endl;
    std::cout << "PLL1 P Output: " << clocks.pll1_p << " Hz" << std::endl;
    // ... and so on for other snapshot members

    return 0;
}
```

**Important:** This library *calculates* frequencies based on the current state of the RCC (Reset and Clock Control) registers. It does **not** configure the clocks itself. You must use STM32 HAL, LL libraries, or direct register access to set up your desired clock configuration *before* calling these functions.

## 🛠️ Enhancements & Design Choices

*   **`constexpr` for Performance:** All functions are `constexpr`, enabling compile-time evaluation. This means the clock frequencies are computed once during the build process and embedded directly into the firmware, eliminating runtime calculations and potential overhead.
*   **`static const uint8_t` Tables:** Decode tables are implemented as `static const` arrays within `constexpr` functions. This ensures they are also evaluated at compile-time and their data is placed in read-only memory, where appropriate.
*   **Defensive Programming:** Checks for division by zero (`div == 0u`) and invalid bitfield values are included where necessary.
*   **Modularity:** Functions are broken down logically (e.g., `decode_prescalers`, `pll_source_hz`, `pllx_p_hz`, etc.), making the code readable and maintainable.
*   **Namespace Usage:** Encapsulates clock-related functionality within `namespace stm32::clocks` to prevent naming conflicts.
*   **Conditional Compilation (`#if defined(...)`):** Uses preprocessor directives to adapt to variations in CMSIS headers across different STM32H7 sub-family devices, enhancing portability.
*   **`snapshot()` Function:** A convenient way to get all the important clock frequencies in one go, useful for debugging or system initialization checks.

## 📜 License

This project is licensed under the MIT License - see the [LICENSE](LICENSE.md) file for details (or assume MIT if no LICENSE file is present).

## 👨‍💻 Author

*   Mohammad

## 💡 Version History

*   **v1.2 (2026-05-06):**
    *   Added more robust guard checks.
    *   Further leveraged `constexpr` for cleaner computations.
    *   Improved clarity of decode tables and comments.
    *   Added more kernel clock calculations (SPI, USART, I2C, RNG).

---