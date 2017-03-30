#ifndef _pwm_h_
#define _pwm_h_

#define n_channels 6
extern uint16_t pwm_duty_width[n_channels];
extern uint16_t pwm_cycle_width[n_channels];
extern void pwm_init();
#endif
