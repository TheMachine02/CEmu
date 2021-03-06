#ifndef FLASH_H
#define FLASH_H

#ifdef __cplusplus
extern "C" {
#endif

#include "port.h"

#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>

/* Standard FLASH state */
typedef struct flash_state {
    uint8_t ports[0x100];
    uint32_t waitStates;
    uint32_t mask;
    uint8_t mapped : 1;
    uint8_t map    : 4;
} flash_state_t;

/* Global flash state */
extern flash_state_t flash;

/* Avbailable functions */
eZ80portrange_t init_flash(void);

/* Save/Restore */
bool flash_restore(FILE *image);
bool flash_save(FILE *image);

#ifdef __cplusplus
}
#endif

#endif
