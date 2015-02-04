/*
 * pwm.h
 *
 * Created: 12/04/2014 16:14:55
 *  Author: limpkin
 */


#ifndef PWM_H_
#define PWM_H_

// Prototypes
void setPwmDc(uint16_t pwm_value);
void initPwm(void);

// Defines
#define MAX_PWM_VAL     0x7FF

#endif /* PWM_H_ */