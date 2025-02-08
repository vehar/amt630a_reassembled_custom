#ifndef VIDEO_ADJUSTMENTS_H
#define VIDEO_ADJUSTMENTS_H

#include <stdint.h>

// Adjust artifact settings based on signal detect registers.
void firm_timer_adjust_artifacts(void);

// Adjust sharpness based on ADC/statistics.
void firm_timer_adjust_sharpness(void);

// Adjust sensitivity (signal strength) based on ADC readings.
void firm_timer_adjust_sensitivity(void);

// Adjust boldness (contrast or “boldness” of image) settings.
void firm_timer_adjust_boldness(void);

// Enter and release “coarse” mode (a fallback mode when video is poor).
void firm_timer_coarse_enter(void);
void firm_timer_coarse_release(void);

// Detect and adjust the framerate/tint settings based on the video signal.
void firm_timer_detect_framerate(void);

// Select the AV input (toggle between AV1/AV2) according to signal detect.
void input_selector(void);

#endif // VIDEO_ADJUSTMENTS_H
