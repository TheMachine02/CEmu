#ifndef MISC_H
#define MISC_H

#ifdef __cplusplus
extern "C" {
#endif

#include "port.h"

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

typedef struct watchdog_state {
    uint32_t count;              /* Standard WATCHDOG state */
    uint32_t load;
    uint16_t restart;
    uint32_t control;
    uint32_t status;
    uint32_t length;
    uint32_t revision;
} watchdog_state_t;

typedef struct protected_state {  /* Standard PROTECTED state */
    bool locked;
    uint8_t led;
    uint8_t ports[0x100];
} protected_state_t;

typedef struct cxxx_state {
    uint8_t ports[0x100];         /* Standard CXXX state */
} cxxx_state_t;
typedef struct exxx_state {
    uint8_t ports[0x80];          /* Standard EXXX state */
} exxx_state_t;
typedef struct fxxx_state {       /* Standard FXXX state */
    uint8_t dummy;                /* Silence warning, remove if other fields are added. */
} fxxx_state_t;

extern watchdog_state_t watchdog;   /* Global WATCHDOG state */
extern protected_state_t protect;   /* Global PROTECT state */
extern cxxx_state_t cxxx;           /* Global CXXX state */
extern exxx_state_t exxx;           /* Global EXXX state */
extern fxxx_state_t fxxx;           /* Global FXXX state */

/* Available functions */
void watchdog_reset(void);
eZ80portrange_t init_watchdog(void);
eZ80portrange_t init_protected(void);
eZ80portrange_t init_cxxx(void);
eZ80portrange_t init_exxx(void);
eZ80portrange_t init_fxxx(void);

/* Save/Restore */
bool watchdog_restore(FILE *image);
bool watchdog_save(FILE *image);
bool protect_restore(FILE *image);
bool protect_save(FILE *image);
bool cxxx_restore(FILE *image);
bool cxxx_save(FILE *image);
bool exxx_restore(FILE *image);
bool exxx_save(FILE *image);

#ifdef __cplusplus
}
#endif

#endif
