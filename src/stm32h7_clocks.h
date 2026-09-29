#pragma once
/**
 * @file stm32h7_clocks_h743_h723.h
 * @brief Clock computation helpers for STM32H743 and STM32H723 (CMSIS-only).
 * @version 1.2
 * @date 2026-05-06
 * @author Mohammad
 * @license MIT
 *
 * Enhancements:
 * - More robust guard checks.
 * - Use of constexpr.
 * - Clearer decode tables.
 * - Additional comments.
 */

#include <stdint.h>
#include "stm32h7xx.h"

namespace stm32 {
namespace clocks {

/* ----------------- Basic helpers ----------------- */

constexpr uint32_t div_pow2(uint32_t shift) { return (shift >= 31u) ? 0u : (1u << shift); }

/* ----------------- Prescaler decode (H7) ----------------- */

constexpr uint32_t decode_d1cpre_div(uint32_t d1cpre_bits)
{
    static const uint8_t shift_tbl[16] = {
        0,0,0,0,  // 0..3: /1
        1,2,3,4,  // 4..7: /2..../16
        1,2,3,4,  // 8..11: /2..../16
        6,7,8,9   // 12..15: /64..../512
    };
    return div_pow2(shift_tbl[d1cpre_bits & 0xFu]);
}

constexpr uint32_t decode_hpre_div(uint32_t hpre_bits)
{
    if ((hpre_bits & 0x8u) == 0u) return 1u;
    switch (hpre_bits & 0xFu) {
        case 0x8: return 2u;
        case 0x9: return 4u;
        case 0xA: return 8u;
        case 0xB: return 16u;
        case 0xC: return 64u;
        case 0xD: return 128u;
        case 0xE: return 256u;
        case 0xF: return 512u;
        default:  return 1u;
    }
}

constexpr uint32_t decode_ppre_div(uint32_t ppre_bits)
{
    if ((ppre_bits & 0x4u) == 0u) return 1u;
    switch (ppre_bits & 0x7u) {
        case 0x4: return 2u;
        case 0x5: return 4u;
        case 0x6: return 8u;
        case 0x7: return 16u;
        default:  return 1u;
    }
}

/* ----------------- Oscillators ----------------- */

constexpr uint32_t hsi_hz(void)
{
    uint32_t divider = 1u;
#if defined(RCC_CR_HSIDIV) && defined(RCC_CR_HSIDIV_Pos)
    /* H743: HSIDIV encoding is 0=/1, 1=/2, 2=/4, 3=/8. */
    switch ((RCC->CR & RCC_CR_HSIDIV) >> RCC_CR_HSIDIV_Pos) {
        case 1u: divider = 2u; break;
        case 2u: divider = 4u; break;
        case 3u: divider = 8u; break;
        default: break;
    }
#endif
    return ((uint32_t)HSI_VALUE / divider);
}

constexpr uint32_t csi_hz(void)
{
#if defined(CSI_VALUE)
    return (uint32_t)CSI_VALUE;
#else
    return 0u;
#endif
}

constexpr uint32_t hse_hz(void)
{
    return (uint32_t)HSE_VALUE;
}

constexpr uint32_t lse_hz(void)
{
#if defined(LSE_VALUE)
    return (uint32_t)LSE_VALUE;
#else
    return 32768u;
#endif
}

/* ----------------- PLL source selection ----------------- */

constexpr uint32_t pll_src_hz(void)
{
#if defined(RCC_PLLCKSELR_PLLSRC)
    uint32_t src = (RCC->PLLCKSELR & RCC_PLLCKSELR_PLLSRC);
    if (src == RCC_PLLCKSELR_PLLSRC_HSI) return hsi_hz();
#if defined(RCC_PLLCKSELR_PLLSRC_CSI)
    if (src == RCC_PLLCKSELR_PLLSRC_CSI) return csi_hz();
#endif
    if (src == RCC_PLLCKSELR_PLLSRC_HSE) return hse_hz();
    return 0u;
#else
    return hsi_hz();
#endif
}

/* ----------------- Generic PLLx decode (x=1,2,3) ----------------- */

constexpr uint32_t pll_m(uint32_t pll_index)
{
    uint32_t reg = RCC->PLLCKSELR;

#if defined(RCC_PLLCKSELR_DIVM1)
    if (pll_index == 1u) return (reg & RCC_PLLCKSELR_DIVM1) >> RCC_PLLCKSELR_DIVM1_Pos;
#endif
#if defined(RCC_PLLCKSELR_DIVM2)
    if (pll_index == 2u) return (reg & RCC_PLLCKSELR_DIVM2) >> RCC_PLLCKSELR_DIVM2_Pos;
#endif
#if defined(RCC_PLLCKSELR_DIVM3)
    if (pll_index == 3u) return (reg & RCC_PLLCKSELR_DIVM3) >> RCC_PLLCKSELR_DIVM3_Pos;
#endif
    return 0u;
}

constexpr void pll_n_p_q_r(uint32_t pll_index, uint32_t &n, uint32_t &p, uint32_t &q, uint32_t &r)
{
    n = p = q = r = 0u;

    if (pll_index == 1u) {
#if defined(RCC_PLL1DIVR_N1)
        uint32_t divr = RCC->PLL1DIVR;
        n = ((divr & RCC_PLL1DIVR_N1) >> RCC_PLL1DIVR_N1_Pos) + 1u;
        p = ((divr & RCC_PLL1DIVR_P1) >> RCC_PLL1DIVR_P1_Pos) + 1u;
        q = ((divr & RCC_PLL1DIVR_Q1) >> RCC_PLL1DIVR_Q1_Pos) + 1u;
        r = ((divr & RCC_PLL1DIVR_R1) >> RCC_PLL1DIVR_R1_Pos) + 1u;
#endif
        return;
    }

    if (pll_index == 2u) {
#if defined(RCC_PLL2DIVR_N2)
        uint32_t divr = RCC->PLL2DIVR;
        n = ((divr & RCC_PLL2DIVR_N2) >> RCC_PLL2DIVR_N2_Pos) + 1u;
        p = ((divr & RCC_PLL2DIVR_P2) >> RCC_PLL2DIVR_P2_Pos) + 1u;
        q = ((divr & RCC_PLL2DIVR_Q2) >> RCC_PLL2DIVR_Q2_Pos) + 1u;
        r = ((divr & RCC_PLL2DIVR_R2) >> RCC_PLL2DIVR_R2_Pos) + 1u;
#endif
        return;
    }

    if (pll_index == 3u) {
#if defined(RCC_PLL3DIVR_N3)
        uint32_t divr = RCC->PLL3DIVR;
        n = ((divr & RCC_PLL3DIVR_N3) >> RCC_PLL3DIVR_N3_Pos) + 1u;
        p = ((divr & RCC_PLL3DIVR_P3) >> RCC_PLL3DIVR_P3_Pos) + 1u;
        q = ((divr & RCC_PLL3DIVR_Q3) >> RCC_PLL3DIVR_Q3_Pos) + 1u;
        r = ((divr & RCC_PLL3DIVR_R3) >> RCC_PLL3DIVR_R3_Pos) + 1u;
#endif
        return;
    }
}

constexpr uint32_t pllx_vco_hz(uint32_t pll_index)
{
    uint32_t fin = pll_src_hz();
    uint32_t m   = pll_m(pll_index);
    uint32_t n, p, q, r;
    pll_n_p_q_r(pll_index, n, p, q, r);
    (void)p; (void)q; (void)r;

    if (fin == 0u || m == 0u || n == 0u) return 0u;

    /* H7 PLLs have a 13-bit optional fractional N divider.  Use 64-bit
       arithmetic so the intermediate product cannot overflow at H7 rates. */
    uint32_t fracn = 0u;
#if defined(RCC_PLLCFGR_PLL1FRACEN) && defined(RCC_PLL1FRACR_FRACN1)
    if (pll_index == 1u && (RCC->PLLCFGR & RCC_PLLCFGR_PLL1FRACEN))
        fracn = (RCC->PLL1FRACR & RCC_PLL1FRACR_FRACN1) >> RCC_PLL1FRACR_FRACN1_Pos;
#endif
#if defined(RCC_PLLCFGR_PLL2FRACEN) && defined(RCC_PLL2FRACR_FRACN2)
    if (pll_index == 2u && (RCC->PLLCFGR & RCC_PLLCFGR_PLL2FRACEN))
        fracn = (RCC->PLL2FRACR & RCC_PLL2FRACR_FRACN2) >> RCC_PLL2FRACR_FRACN2_Pos;
#endif
#if defined(RCC_PLLCFGR_PLL3FRACEN) && defined(RCC_PLL3FRACR_FRACN3)
    if (pll_index == 3u && (RCC->PLLCFGR & RCC_PLLCFGR_PLL3FRACEN))
        fracn = (RCC->PLL3FRACR & RCC_PLL3FRACR_FRACN3) >> RCC_PLL3FRACR_FRACN3_Pos;
#endif
    const uint64_t n_scaled = ((uint64_t)n * 8192u) + fracn;
    return (uint32_t)(((uint64_t)fin * n_scaled) / ((uint64_t)m * 8192u));
}

constexpr uint32_t pllx_p_hz(uint32_t pll_index)
{
    uint32_t n, p, q, r;
    pll_n_p_q_r(pll_index, n, p, q, r);
    (void)n; (void)q; (void)r;
    uint32_t vco = pllx_vco_hz(pll_index);
    return (p == 0u) ? 0u : (vco / p);
}

constexpr uint32_t pllx_q_hz(uint32_t pll_index)
{
    uint32_t n, p, q, r;
    pll_n_p_q_r(pll_index, n, p, q, r);
    (void)n; (void)p; (void)r;
    uint32_t vco = pllx_vco_hz(pll_index);
    return (q == 0u) ? 0u : (vco / q);
}

constexpr uint32_t pllx_r_hz(uint32_t pll_index)
{
    uint32_t n, p, q, r;
    pll_n_p_q_r(pll_index, n, p, q, r);
    (void)n; (void)p; (void)q;
    uint32_t vco = pllx_vco_hz(pll_index);
    return (r == 0u) ? 0u : (vco / r);
}

/* Convenience wrappers */
constexpr uint32_t pll1_vco_hz(void) { return pllx_vco_hz(1u); }
constexpr uint32_t pll1_p_hz(void)   { return pllx_p_hz(1u); }
constexpr uint32_t pll1_q_hz(void)   { return pllx_q_hz(1u); }
constexpr uint32_t pll1_r_hz(void)   { return pllx_r_hz(1u); }

constexpr uint32_t pll2_p_hz(void)   { return pllx_p_hz(2u); }
constexpr uint32_t pll2_q_hz(void)   { return pllx_q_hz(2u); }
constexpr uint32_t pll2_r_hz(void)   { return pllx_r_hz(2u); }

constexpr uint32_t pll3_p_hz(void)   { return pllx_p_hz(3u); }
constexpr uint32_t pll3_q_hz(void)   { return pllx_q_hz(3u); }
constexpr uint32_t pll3_r_hz(void)   { return pllx_r_hz(3u); }

/* ----------------- SYSCLK / core clocks ----------------- */

constexpr uint32_t sysclk_hz(void)
{
#if defined(RCC_CFGR_SWS)
    uint32_t sws = (RCC->CFGR & RCC_CFGR_SWS) >> RCC_CFGR_SWS_Pos;
    switch (sws) {
        case 0x0: return hsi_hz();     /* HSI */
        case 0x1: return csi_hz();     /* CSI */
        case 0x2: return hse_hz();     /* HSE */
        case 0x3: return pll1_p_hz();  /* PLL1 */
        default:  return 0u;
    }
#else
    return hsi_hz();
#endif
}

constexpr uint32_t cpuclk_hz(void)
{
#if defined(RCC_D1CFGR_D1CPRE)
    uint32_t div = decode_d1cpre_div((RCC->D1CFGR & RCC_D1CFGR_D1CPRE) >> RCC_D1CFGR_D1CPRE_Pos);
    uint32_t sys = sysclk_hz();
    return (div == 0u) ? 0u : (sys / div);
#else
    return sysclk_hz();
#endif
}

constexpr uint32_t hclk_hz(void)
{
#if defined(RCC_D1CFGR_HPRE)
    uint32_t div = decode_hpre_div((RCC->D1CFGR & RCC_D1CFGR_HPRE) >> RCC_D1CFGR_HPRE_Pos);
    uint32_t cpu = cpuclk_hz();
    return (div == 0u) ? 0u : (cpu / div);
#else
    return cpuclk_hz();
#endif
}

/* ----------------- APB clocks ----------------- */

constexpr uint32_t pclk3_hz(void)
{
#if defined(RCC_D1CFGR_D1PPRE)
    uint32_t div = decode_ppre_div((RCC->D1CFGR & RCC_D1CFGR_D1PPRE) >> RCC_D1CFGR_D1PPRE_Pos);
    uint32_t h   = hclk_hz();
    return (div == 0u) ? 0u : (h / div);
#else
    return 0u;
#endif
}

constexpr uint32_t pclk1_hz(void)
{
#if defined(RCC_D2CFGR_D2PPRE1)
    uint32_t div = decode_ppre_div((RCC->D2CFGR & RCC_D2CFGR_D2PPRE1) >> RCC_D2CFGR_D2PPRE1_Pos);
    uint32_t h   = hclk_hz();
    return (div == 0u) ? 0u : (h / div);
#else
    return 0u;
#endif
}

constexpr uint32_t pclk2_hz(void)
{
#if defined(RCC_D2CFGR_D2PPRE2)
    uint32_t div = decode_ppre_div((RCC->D2CFGR & RCC_D2CFGR_D2PPRE2) >> RCC_D2CFGR_D2PPRE2_Pos);
    uint32_t h   = hclk_hz();
    return (div == 0u) ? 0u : (h / div);
#else
    return 0u;
#endif
}

constexpr uint32_t pclk4_hz(void)
{
#if defined(RCC_D3CFGR_D3PPRE)
    uint32_t div = decode_ppre_div((RCC->D3CFGR & RCC_D3CFGR_D3PPRE) >> RCC_D3CFGR_D3PPRE_Pos);
    uint32_t h   = hclk_hz();
    return (div == 0u) ? 0u : (h / div);
#else
    return 0u;
#endif
}

/* ----------------- Timer clocks ----------------- */

<<<<<<< Updated upstream
constexpr uint32_t tim_apb1_hz(void)
=======
static inline uint32_t timer_kernel_hz(uint32_t pclk, uint32_t apb_div)
{
#if defined(RCC_CFGR_TIMPRE)
    if ((RCC->CFGR & RCC_CFGR_TIMPRE) != 0u)
        return (apb_div <= 4u) ? hclk_hz() : (pclk * 4u);
#endif
    return (apb_div == 1u) ? pclk : (pclk * 2u);
}

static inline uint32_t tim_apb1_hz(void)
>>>>>>> Stashed changes
{
#if defined(RCC_D2CFGR_D2PPRE1)
    uint32_t div = decode_ppre_div((RCC->D2CFGR & RCC_D2CFGR_D2PPRE1) >> RCC_D2CFGR_D2PPRE1_Pos);
    return timer_kernel_hz(pclk1_hz(), div);
#else
    return 0u;
#endif
}

constexpr uint32_t tim_apb2_hz(void)
{
#if defined(RCC_D2CFGR_D2PPRE2)
    uint32_t div = decode_ppre_div((RCC->D2CFGR & RCC_D2CFGR_D2PPRE2) >> RCC_D2CFGR_D2PPRE2_Pos);
    return timer_kernel_hz(pclk2_hz(), div);
#else
    return 0u;
#endif
}

constexpr uint32_t tim_apb4_hz(void)
{
#if defined(RCC_D3CFGR_D3PPRE)
    uint32_t div = decode_ppre_div((RCC->D3CFGR & RCC_D3CFGR_D3PPRE) >> RCC_D3CFGR_D3PPRE_Pos);
    return timer_kernel_hz(pclk4_hz(), div);
#else
    return 0u;
#endif
}

/* ----------------- Kernel clocks (mux-aware for H743/H723) ----------------- */

constexpr uint32_t perck_hz(void)
{
#if defined(RCC_D1CCIPR_CKPERSEL)
    /* CKPERSEL: 0=HSI, 1=CSI, 2=HSE. */
    switch ((RCC->D1CCIPR & RCC_D1CCIPR_CKPERSEL) >> RCC_D1CCIPR_CKPERSEL_Pos) {
        case 1u: return csi_hz();
        case 2u: return hse_hz();
        default: return hsi_hz();
    }
#else
    return hsi_hz();
#endif
}

/* SPI123 kernel clock from RCC_D2CCIP1R.SPI123SEL */
constexpr uint32_t spi123_kernel_hz(void)
{
#if defined(RCC_D2CCIP1R_SPI123SEL)
    uint32_t sel = (RCC->D2CCIP1R & RCC_D2CCIP1R_SPI123SEL) >> RCC_D2CCIP1R_SPI123SEL_Pos;
    /* Values: PLL1Q / PLL2P / PLL3P / I2SCKIN / PERCK :contentReference[oaicite:2]{index=2} */
    switch (sel) {
        case 0u: return pll1_q_hz();   /* PLL1Q */
        case 1u: return pll2_p_hz();   /* PLL2P */
        case 2u: return pll3_p_hz();   /* PLL3P */
        case 3u: return 0u;            /* I2SCKIN pin (unknown in SW) */
        case 4u: return perck_hz();    /* PERCK */
        default: return 0u;
    }
#else
    return 0u;
#endif
}

/* SPI45 kernel clock from RCC_D2CCIP1R.SPI45SEL */
constexpr uint32_t spi45_kernel_hz(void)
{
#if defined(RCC_D2CCIP1R_SPI45SEL)
    uint32_t sel = (RCC->D2CCIP1R & RCC_D2CCIP1R_SPI45SEL) >> RCC_D2CCIP1R_SPI45SEL_Pos;
    /* Values: APB4 / PLL2Q / PLL3Q / HSI / CSI / HSE :contentReference[oaicite:3]{index=3} */
    switch (sel) {
        case 0u: return pclk4_hz();    /* APB4 */
        case 1u: return pll2_q_hz();   /* PLL2Q */
        case 2u: return pll3_q_hz();   /* PLL3Q */
        case 3u: return hsi_hz();      /* HSI */
        case 4u: return csi_hz();      /* CSI */
        case 5u: return hse_hz();      /* HSE */
        default: return 0u;
    }
#else
    return 0u;
#endif
}

constexpr uint32_t spi_kernel_hz(SPI_TypeDef* spi)
{
    if (!spi) return 0u;

#if defined(SPI1_BASE)
    if (spi == SPI1) return spi123_kernel_hz();
#endif
#if defined(SPI2_BASE)
    if (spi == SPI2) return spi123_kernel_hz();
#endif
#if defined(SPI3_BASE)
    if (spi == SPI3) return spi123_kernel_hz();
#endif
#if defined(SPI4_BASE)
    if (spi == SPI4) return spi45_kernel_hz();
#endif
#if defined(SPI5_BASE)
    if (spi == SPI5) return spi45_kernel_hz();
#endif
#if defined(SPI6_BASE)
    if (spi == SPI6) return pclk4_hz(); /* SPI6 usually on APB4 (no SPI6SEL here) */
#endif
    return 0u;
}

/* Named wrappers shared with the STM32F4 clock API. */
static inline uint32_t spi1_kernel_hz(void) { return spi_kernel_hz(SPI1); }
static inline uint32_t spi2_kernel_hz(void) { return spi_kernel_hz(SPI2); }
static inline uint32_t spi3_kernel_hz(void) { return spi_kernel_hz(SPI3); }
static inline uint32_t spi4_kernel_hz(void) { return spi_kernel_hz(SPI4); }
static inline uint32_t spi5_kernel_hz(void) { return spi_kernel_hz(SPI5); }
static inline uint32_t spi6_kernel_hz(void) { return spi_kernel_hz(SPI6); }

/* Actual SPI SCK after the H7 SPI baud-rate prescaler. */
static inline uint32_t spi_sck_hz(SPI_TypeDef* spi)
{
    if (!spi) return 0u;
#if defined(SPI1_BASE)
    if (spi == SPI1 || spi == SPI2 || spi == SPI3 ||
        spi == SPI4 || spi == SPI5 || spi == SPI6) {
        uint32_t mbr = (spi->CFG1 & SPI_CFG1_MBR) >> SPI_CFG1_MBR_Pos;
        uint32_t divisor = 2u << (mbr & 0x7u); /* /2, /4, ... /256 */
        uint32_t kernel = spi_kernel_hz(spi);
        return (divisor == 0u) ? 0u : (kernel / divisor);
    }
#endif
    return 0u;
}

/* FDCAN1/FDCAN2 share one kernel-clock selector on STM32H743. */
static inline uint32_t fdcan_kernel_hz(void)
{
#if defined(RCC_D2CCIP1R_FDCANSEL)
    switch ((RCC->D2CCIP1R & RCC_D2CCIP1R_FDCANSEL) >> RCC_D2CCIP1R_FDCANSEL_Pos) {
        case 0u: return hse_hz();
        case 1u: return pll1_q_hz();
        case 2u: return pll2_q_hz();
        default: return 0u;
    }
#else
    return 0u;
#endif
}

/* USART kernel clock selection values are the same enum for both groups. :contentReference[oaicite:4]{index=4} */
constexpr uint32_t usart_sel_to_hz(uint32_t usartsel, uint32_t pclk_hz_in)
{
    switch (usartsel) {
        case 0u: return pclk_hz_in;  /* PCLK */
        case 1u: return pll2_q_hz(); /* PLL2Q */
        case 2u: return pll3_q_hz(); /* PLL3Q */
        case 3u: return hsi_hz();    /* HSI */
        case 4u: return csi_hz();    /* CSI */
        case 5u: return lse_hz();    /* LSE */
        default: return 0u;
    }
}

constexpr uint32_t usart16_kernel_hz(void)
{
#if defined(RCC_D2CCIP2R_USART16SEL)
    uint32_t sel = (RCC->D2CCIP2R & RCC_D2CCIP2R_USART16SEL) >> RCC_D2CCIP2R_USART16SEL_Pos;
    return usart_sel_to_hz(sel, pclk2_hz());
#else
    return pclk2_hz();
#endif
}

constexpr uint32_t usart234578_kernel_hz(void)
{
#if defined(RCC_D2CCIP2R_USART234578SEL)
    uint32_t sel = (RCC->D2CCIP2R & RCC_D2CCIP2R_USART234578SEL) >> RCC_D2CCIP2R_USART234578SEL_Pos;
    return usart_sel_to_hz(sel, pclk1_hz());
#else
    return pclk1_hz();
#endif
}

constexpr uint32_t usart_kernel_hz(USART_TypeDef* u)
{
    if (!u) return 0u;

#if defined(USART1_BASE)
    if (u == USART1) return usart16_kernel_hz();
#endif
#if defined(USART6_BASE)
    if (u == USART6) return usart16_kernel_hz();
#endif
#if defined(UART9_BASE)
    if (u == UART9)  return usart16_kernel_hz();
#endif
#if defined(USART10_BASE)
    if (u == USART10) return usart16_kernel_hz();
#endif

#if defined(USART2_BASE)
    if (u == USART2) return usart234578_kernel_hz();
#endif
#if defined(USART3_BASE)
    if (u == USART3) return usart234578_kernel_hz();
#endif
#if defined(UART4_BASE)
    if (u == UART4)  return usart234578_kernel_hz();
#endif
#if defined(UART5_BASE)
    if (u == UART5)  return usart234578_kernel_hz();
#endif
#if defined(UART7_BASE)
    if (u == UART7)  return usart234578_kernel_hz();
#endif
#if defined(UART8_BASE)
    if (u == UART8)  return usart234578_kernel_hz();
#endif

    return 0u;
}

/* Named wrappers shared with the STM32F4 clock API. */
static inline uint32_t usart1_kernel_hz(void) { return usart_kernel_hz(USART1); }
static inline uint32_t usart2_kernel_hz(void) { return usart_kernel_hz(USART2); }
static inline uint32_t usart3_kernel_hz(void) { return usart_kernel_hz(USART3); }
static inline uint32_t uart4_kernel_hz(void)  { return usart_kernel_hz(UART4); }
static inline uint32_t uart5_kernel_hz(void)  { return usart_kernel_hz(UART5); }
static inline uint32_t usart6_kernel_hz(void) { return usart_kernel_hz(USART6); }
static inline uint32_t uart7_kernel_hz(void)  { return usart_kernel_hz(UART7); }
static inline uint32_t uart8_kernel_hz(void)  { return usart_kernel_hz(UART8); }

/* I2C123SEL: 0=PCLK1, 1=PLL3R, 2=HSI, 3=CSI :contentReference[oaicite:5]{index=5} */
constexpr uint32_t i2c123_kernel_hz(void)
{
#if defined(RCC_D2CCIP2R_I2C123SEL)
    uint32_t sel = (RCC->D2CCIP2R & RCC_D2CCIP2R_I2C123SEL) >> RCC_D2CCIP2R_I2C123SEL_Pos;
    switch (sel) {
        case 0u: return pclk1_hz();
        case 1u: return pll3_r_hz();
        case 2u: return hsi_hz();
        case 3u: return csi_hz();
        default: return 0u;
    }
#else
    return pclk1_hz();
#endif
}

constexpr uint32_t i2c_kernel_hz(I2C_TypeDef* i)
{
    if (!i) return 0u;

#if defined(I2C1_BASE)
    if (i == I2C1) return i2c123_kernel_hz();
#endif
#if defined(I2C2_BASE)
    if (i == I2C2) return i2c123_kernel_hz();
#endif
#if defined(I2C3_BASE)
    if (i == I2C3) return i2c123_kernel_hz();
#endif
#if defined(I2C4_BASE)
    if (i == I2C4) return pclk4_hz(); /* I2C4 mux exists on some parts (D3CCIPR), not handled here */
#endif
    return 0u;
}

/* Named wrappers shared with the STM32F4 clock API. */
static inline uint32_t i2c1_kernel_hz(void) { return i2c_kernel_hz(I2C1); }
static inline uint32_t i2c2_kernel_hz(void) { return i2c_kernel_hz(I2C2); }
static inline uint32_t i2c3_kernel_hz(void) { return i2c_kernel_hz(I2C3); }
static inline uint32_t i2c4_kernel_hz(void) { return i2c_kernel_hz(I2C4); }

/* RNGSEL: HSI48 / PLL1Q / LSE / LSI :contentReference[oaicite:6]{index=6} */
constexpr uint32_t rng_kernel_hz(void)
{
#if defined(RCC_D2CCIP2R_RNGSEL)
    uint32_t sel = (RCC->D2CCIP2R & RCC_D2CCIP2R_RNGSEL) >> RCC_D2CCIP2R_RNGSEL_Pos;
    switch (sel) {
        case 0u:
#if defined(HSI48_VALUE)
            return (uint32_t)HSI48_VALUE;
#else
            return 48000000u;
#endif
        case 1u: return pll1_q_hz();
        case 2u: return lse_hz();
        case 3u:
#if defined(LSI_VALUE)
            return (uint32_t)LSI_VALUE;
#else
            return 32000u;
#endif
        default: return 0u;
    }
#else
    return 0u;
#endif
}

/* ----------------- Snapshot ----------------- */

struct ClockSnapshot {
    uint32_t sysclk, cpuclk, hclk;
    uint32_t pclk1, pclk2, pclk3, pclk4;
    uint32_t tim_apb1, tim_apb2, tim_apb4;

    uint32_t pll1_vco, pll1_p, pll1_q, pll1_r;
    uint32_t pll2_p, pll2_q, pll2_r;
    uint32_t pll3_p, pll3_q, pll3_r;

    uint32_t spi123_ker, spi45_ker;
    uint32_t usart16_ker, usart234578_ker;
    uint32_t i2c123_ker;
    uint32_t rng_ker;
};

constexpr ClockSnapshot snapshot(void)
{
    ClockSnapshot s{};
    s.sysclk = sysclk_hz();
    s.cpuclk = cpuclk_hz();
    s.hclk   = hclk_hz();

    s.pclk1  = pclk1_hz();
    s.pclk2  = pclk2_hz();
    s.pclk3  = pclk3_hz();
    s.pclk4  = pclk4_hz();

    s.tim_apb1 = tim_apb1_hz();
    s.tim_apb2 = tim_apb2_hz();
    s.tim_apb4 = tim_apb4_hz();

    s.pll1_vco = pll1_vco_hz();
    s.pll1_p   = pll1_p_hz();
    s.pll1_q   = pll1_q_hz();
    s.pll1_r   = pll1_r_hz();

    s.pll2_p   = pll2_p_hz();
    s.pll2_q   = pll2_q_hz();
    s.pll2_r   = pll2_r_hz();

    s.pll3_p   = pll3_p_hz();
    s.pll3_q   = pll3_q_hz();
    s.pll3_r   = pll3_r_hz();

    s.spi123_ker = spi123_kernel_hz();
    s.spi45_ker  = spi45_kernel_hz();
    s.usart16_ker = usart16_kernel_hz();
    s.usart234578_ker = usart234578_kernel_hz();
    s.i2c123_ker = i2c123_kernel_hz();
    s.rng_ker    = rng_kernel_hz();
    return s;
}

}} // namespace stm32::clocks
