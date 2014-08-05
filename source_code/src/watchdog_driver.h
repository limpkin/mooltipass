#include <avr/io.h>
#include <avr/wdt.h>

#define wdt_stop()          (WDTCSR = 0x00)                                     // Stop the watchdog timer
#define wdt_clear_flag()    (MCUSR&= ~(1<<WDRF))                                // Clear flag
#define wdt_enable_8s()     (WDTCSR = (1<<WDE)|(1<<WDP3)|(1<<WDP0))             // Enable a 8s watchdog reset
#define wdt_enable_2s()     (WDTCSR = (1<<WDE)|(1<<WDP2)|(1<<WDP1)|(1<<WDP0))   // Enable a 2s watchdog reset
#define wdt_enable_16ms()   (WDTCSR = (1<<WDE))                                 // Enable a 16ms watchdog reset
#define is_wdt_reset()      ((MCUSR&(1<<WDRF)) ? 1:0)                           // Is it a watchdog interrupt that made the reset ?
#define wdt_change_enable() (WDTCSR |= (1<<WDCE) | (1<<WDE))                    // Enable future changes on wdtcsr
