#ifndef PWM_H
#define PWM_H

#include <cstdint>

#include "gpio.h"

/* Init
 *
 * GCLK for TCC
 */

// KessilPwm(GROUP_A, 17, 16);

/**
16 -> TCC0/WO[5] -> CC1
17 -> TCC0/WO[6] -> CC2
**/

class KessilPwm {
    constexpr auto intensity_pin = 16;
    constexpr auto spectrum_pin = 17;
public:
    Pwm() {
        PORT->Group[GROUP_A].PMUX[spectrum_pin / 2].bit.PMUXE = F;
        PORT->Group[GROUP_A].PMUX[intensity_pin / 2].bit.PMUXO = F;

        PORT->Group[GROUP_A].PINCFG[spectrum_pin].bit.PMUXEN = 1;
        PORT->Group[GROUP_A].PINCFG[intensity_pin].bit.PMUXEN = 1;

        TCCA->CTRLA.bit.PRESCALE = 0;  /* 16 MHz CLK */
        TCCA->PER = 1 << 14;           /* 960 Hz */
        TCCA->WAVE.bit.WAVEGEN = 0x02; /* Normal PWM */
        TCCA->CTRLA.bit.ENABLE = 1;
    }

    void set_spectrum(uint16_t duty) {
        TCCA->CC2 = (uint32_t)duty << 16;
    }

    void set_intensity(uint16_t duty) {
        TCCA->CC1 = (uint32_t)duty << 16;
    }
}

struct KessilPoint {
    uint32_t time;
    uint16_t intensity;
    uint16_t spectrum;
}

class KessilController {
public:
    KessilController()

    void clear_sequence() { sequence.clear(); }
    void add_point(KessilPoint point) {
        sequence.push_back(point);
        std::sort(sequence.begin(), sequence.end(), [](KessilPoint a, KessilPoint b) {
            return a.time > b.time;
        });
    }

    void run(uint32_t time_of_day) {
        int i = find_index(time_of_day);
        next_point = sequence[i];
        prev_point = (i == 0) ? sequence.back() : sequence[i-1];

        auto point = interpolate(time_of_day, prev_point, next_point);

        pwm.set_intensity(point.intensity);
        pwm.set_spectrum(point.spectrum);
    }


private:
    int find(uint32_t time_of_day) {
        for(int i; i<sequence.size(); i++) {
            auto point = sequence[i];
            if( point.time > time_of_day ) {
                return i;
            }
        }
        return 0;
    }

    KessilPoint interpolate(uint32_t time, const KessilPoint& prev, const KessilPoint& next) const {
        return KessilPoint{
            time = time,
            intensity = linear_interp(time, prev.time, next.time, prev.intensity, next.intensity),
            spectrum = linear_interp(time, prev.time, next.time, prev.spectrum, next.spectrum)
        };
    }

    int linear_interp(int x, x0, x1, int y0, y1) {
        x = x / 1000;
        x0 = x0 / 1000;
        x1 = x1 / 1000;
        return (y0 * (x1 - x) + y1 * (x - x0)) / (x1 - x0);
    }

    std::vector<KessilPoint> sequence;
    KessilPwm pwm;


}


#endif  /* PWM_H */
