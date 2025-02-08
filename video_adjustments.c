#include "video_adjustments.h"
#include "hardware.h" // This file must define the register addresses and macros (for example, REG8(addr))
#include <stdint.h>

// For simulation purposes we define some global “XRAM” variables.
// In the original firmware these would be stored in external RAM.
uint8_t iram_tmp_23h = 0;
uint8_t iram_tmp_24h = 0;
uint8_t iram_old_AV_stat_signal_detect = 0xFF;
uint8_t iram_irq_artifacts_offhold = 0;
uint8_t iram_irq_sharpness_offhold = 0;
uint8_t iram_irq_sensitivity_offhold = 0;
uint8_t iram_timer1_irq_counter = 0;
uint8_t iram_palntsc_same_counter = 0;
uint8_t iram_old_AV_framerate = 0;
uint8_t xram_new_detected_video = 0xFF;
uint8_t xram_sett_pal_ntsc = 0; // 0 = NTSC, nonzero = forced PAL60
uint8_t xram_sett_ratio = 0;
uint8_t xram_curr_input = 0; // 0 = AV1; 1 = AV2

//------------------------------------------------------------
// Artifact Adjustment
//------------------------------------------------------------
void firm_timer_adjust_artifacts(void)
{
    // Read the lower nibble of the signal–detection register.
    uint8_t stat = REG8(IO_AV_stat_signal_detect) & 0x0F;
    iram_tmp_23h = stat;
    // Copy the complete byte from the secondary detect register.
    iram_tmp_24h = REG8(IO_AV_stat_detect_2);

    // Read the previous (old) signal detect value.
    uint8_t oldVal = iram_old_AV_stat_signal_detect;
    uint8_t diff = oldVal ^ iram_tmp_23h;
    if (diff == 0)
    {
        // Signal is stable: reset the “artifact offhold” to zero.
        iram_irq_artifacts_offhold = 0;
    }
    else
    {
        // Signal changed – update the offhold counter.
        if (iram_irq_artifacts_offhold < 0xFA)
        {
            iram_irq_artifacts_offhold++;
        }
        // If the offhold counter is still low, force a nonzero value into the artifact–control register.
        if (iram_irq_artifacts_offhold < 0x14)
        {
            REG8(IO_AV_ctrl_artifacts) = 0x06;
        }
        else
        {
            REG8(IO_AV_ctrl_artifacts) = 0x00;
        }
        // Also, update the stored old value.
        iram_old_AV_stat_signal_detect = iram_tmp_23h;
    }
}
void firm_timer_adjust_artifacts(void)
{
    // Copy low nibble of IO_AV_stat_signal_detect into a temporary location.
    unsigned char lsbs = (*(volatile unsigned char *)IO_AV_stat_signal_detect) & 0x0F;
    *(volatile unsigned char *)iram_tmp__23h = lsbs;
    // Copy full byte from IO_AV_stat_detect_2.
    unsigned char detect2 = *(volatile unsigned char *)IO_AV_stat_detect_2;
    *(volatile unsigned char *)iram_tmp__24h = detect2;
    // Get old value from XRAM.
    unsigned char old_val = *(volatile unsigned char *)xram_old_AV_stat_signal_detect;
    unsigned char r7 = old_val;
    if ((old_val ^ lsbs) == 0)
    {
        // Stable signal: check offhold counter.
        unsigned char offhold = *(volatile unsigned char *)xram_irq_artifacts_offhold;
        if (offhold >= 0x14) // Already reached threshold (20 decimal)
            return;

        offhold++;
        *(volatile unsigned char *)xram_irq_artifacts_offhold = offhold;
        return;
    }
    // Signal has changed: update stored value and reset offhold.
    *(volatile unsigned char *)xram_old_AV_stat_signal_detect = lsbs;
    *(volatile unsigned char *)xram_irq_artifacts_offhold = 0;
}
// Adjust “artifacts” based on AV signal detect registers.
void firm_timer_adjust_artifacts(void)
{
    // Copy lower nibble of IO_AV_stat_signal_detect:
    unsigned char new_signal = IO_AV_stat_signal_detect & 0x0F;
    xram_tmp__23h = new_signal;
    // Copy full byte of IO_AV_stat_detect_2:
    xram_tmp__24h = IO_AV_stat_detect_2;

    // Read the old AV stat value (stored in XRAM)
    unsigned char old_signal = xram_old_AV_stat_signal_detect;

    // Compare new lower nibble with old:
    if ((old_signal ^ xram_tmp__23h) != 0)
    {
        // Signal changed. Update old value and reset artifact offhold.
        xram_old_AV_stat_signal_detect = xram_tmp__23h;
        xram_irq_artifacts_offhold = 0;
        return;
    }
    // Otherwise, signal is stable: increment offhold counter.
    if (xram_irq_artifacts_offhold < 0x14)
    { // 20 decimal threshold
        xram_irq_artifacts_offhold++;
    }
    if (xram_irq_artifacts_offhold >= 0x14)
    {
        // When offhold counter reaches threshold, adjust control registers.
        // (Here we use a simplified heuristic based on the old value.)
        if (old_signal < 0x10)
        {
            // If signal is low, clear artifacts.
            IO_AV_ctrl_artifacts = 0;
            // And set bit1 in ctrl_whatever_1:
            IO_AV_ctrl_whatever_1 |= 0x02;
        }
        else
        {
            // Otherwise, set artifacts control to 2 and clear bit1:
            IO_AV_ctrl_artifacts = 0x02;
            IO_AV_ctrl_whatever_1 &= ~0x02;
        }
    }
}

//------------------------------------------------------------
// Sharpness Adjustment
//------------------------------------------------------------
void firm_timer_adjust_sharpness(void)
{
    // Read the second signal-detect register.
    uint8_t newVal = REG8(IO_AV_stat_detect_2);
    if (newVal != iram_tmp_24h)
    {
        // Signal changed: store new value and reset offhold.
        REG8(IO_AV_stat_detect_2) = newVal;
        iram_irq_sharpness_offhold = 0;
    }
    else
    {
        // Otherwise, increment the offhold value (simulate gradual adjustment).
        if (iram_irq_sharpness_offhold < 10)
        {
            iram_irq_sharpness_offhold++;
        }
    }
}

//------------------------------------------------------------
// Sensitivity Adjustment
//------------------------------------------------------------
void firm_timer_adjust_sensitivity(void)
{
    // Read bit1 of IO_AV_stat_detect_0.
    uint8_t stat = REG8(IO_AV_stat_detect_0) & 0x02;
    if (stat == 0)
    {
        // If the sensitivity trigger bit is not set, skip.
        return;
    }
    // Increment the sensitivity offhold counter.
    iram_irq_sensitivity_offhold++;
    if (iram_irq_sensitivity_offhold >= 0x64)
    { // every 100 counts (dummy threshold)
        // Read the current sensitivity from the hardware.
        uint8_t sens_msb = REG8(IO_AV_stat_sensitivity_msb);
        uint8_t sens_lsb = REG8(IO_AV_stat_sensitivity_lsb);
        // If the sensitivity is too low (dummy threshold: 416 and 1)
        if (sens_lsb < 0xA0 || sens_msb < 1)
        {
            // Adjust the sensitivity by writing new values.
            REG8(IO_AV_ctrl_sensitivity_1) = 0xB6;
            REG8(IO_AV_ctrl_sensitivity_0) = 0x09;
        }
    }
}

//------------------------------------------------------------
// Boldness Adjustment
//------------------------------------------------------------
void firm_timer_adjust_boldness(void)
{
    // Read the detection value from IO_AV_stat_detect_1.
    uint8_t detect = REG8(IO_AV_stat_detect_1);
    if ((detect & 0x01) != 0)
    {
        // If bit0 is set, enable boldness by setting bit4 in IO_50HZ_control_lsb.
        REG8(IO_50HZ_control_lsb) |= 0x10;
        REG8(IO_60HZ_control_mid) |= 0x10;
        // Set boldness contrast to 0.
        REG8(IO_60HZ_boldness_contrast) = 0x00;
    }
    else
    {
        // Otherwise, clear the boldness bits.
        REG8(IO_50HZ_control_lsb) &= ~(0x10);
        REG8(IO_60HZ_control_mid) &= ~(0x10);
        REG8(IO_60HZ_boldness_contrast) = 0x02;
    }
    // Mirror the setting to IO_50HZ_boldness_contrast.
    REG8(IO_50HZ_boldness_contrast) = REG8(IO_60HZ_boldness_contrast);
}

//------------------------------------------------------------
// Coarse Mode Enter/Release
//------------------------------------------------------------
void firm_timer_coarse_enter(void)
{
    // Clear the coarse offhold value.
    REG8(xram_irq_coarse_offhold) = 0;
    // Set bit6 (0x40) in IO_AV_ctrl_whatever_2.
    REG8(IO_AV_ctrl_whatever_2) |= 0x40;
}

void firm_timer_coarse_release(void)
{
    // Check a user setting (xram_sett_pal_ntsc) to determine whether to release coarse mode.
    uint8_t palSetting = REG8(xram_sett_pal_ntsc);
    if (palSetting != 0)
    {
        // In PAL60 mode, do not release coarse mode.
        return;
    }
    // Otherwise, read the offhold value.
    uint8_t offhold = REG8(xram_irq_coarse_offhold);
    // If offhold is less than 250 (0xFA), increment it.
    if (offhold < 0xFA)
    {
        offhold++;
        REG8(xram_irq_coarse_offhold) = offhold;
    }
    // When offhold reaches 250, clear bit6 in IO_AV_ctrl_whatever_2.
    if (offhold == 0xFA)
    {
        REG8(IO_AV_ctrl_whatever_2) &= ~(0x40);
    }
}

//------------------------------------------------------------
// Framerate Detection
//------------------------------------------------------------
void firm_timer_detect_framerate(void)
{
    // Read IO_AV_stat_detect_0 and mask bits 1-2.
    uint8_t stat = REG8(IO_AV_stat_detect_0) & 0x06;
    // If the resulting value equals 0x06, the signal is valid.
    if ((stat ^ 0x06) == 0)
    {
        // Check if a new detected video flag is already set.
        if (REG8(xram_new_detected_video) == 0xFF)
        {
            // Copy the detected framerate from the old value.
            REG8(xram_new_detected_video) = REG8(iram_old_AV_framerate);
        }
    }
    else
    {
        // Signal is not valid: reset the PAL/NTSC same counter.
        REG8(xram_palntsc_same_counter) = 0;
    }
}
void firm_timer_detect_framerate(void)
{
    unsigned char detect0 = *(volatile unsigned char *)IO_AV_stat_detect_0;
    if ((detect0 & 0x06) != 0)
    {
        // Signal valid.
        unsigned char flag = *(volatile unsigned char *)IO_AV_stat_framerate_flag & 0x04;
        unsigned char old_flag = *(volatile unsigned char *)iram_old_AV_framerate;
        if (flag == old_flag)
        {
            unsigned char same_counter = *(volatile unsigned char *)xram_palntsc_same_counter;
            if (same_counter < 0x14)
            {
                same_counter++;
                *(volatile unsigned char *)xram_palntsc_same_counter = same_counter;
                return;
            }
        }
    }
    // Otherwise, reset SAME counter and update detected video signal.
    *(volatile unsigned char *)xram_palntsc_same_counter = 0;
    *(volatile unsigned char *)xram_new_detected_video = *(volatile unsigned char *)iram_old_AV_framerate;
}
//------------------------------------------------------------
// Input Selector
//------------------------------------------------------------
void input_selector(void)
{
    // This routine selects between AV1 and AV2 input.
    // First, check if the PLL (IO_PLL_12h_used) has bit 2 set (powered).
    uint8_t pll = REG8(IO_PLL_12h_used);
    if (pll & 0x04)
    {
        // If PLL is powered, then use the primary input.
        REG8(xram_sett_input) &= ~(0x02); // Clear bit1 (force primary)
    }
    else
    {
        // Otherwise, toggle the current input.
        uint8_t current = REG8(xram_curr_input);
        current ^= 0x01; // Toggle LSB
        REG8(xram_curr_input) = current;
    }
    // Then apply the new input by calling a hardware–dependent routine.
    // (For example, write to AV select registers.)
    // Here we simply call a stub function:
    // apply_av_input_a();  // (Not implemented here.)
}
