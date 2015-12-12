#ifndef EMU_H
#define EMU_H

#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

extern int cycle_count_delta;
extern int throttle_delay;
extern uint32_t cpu_events;
#define EVENT_RESET 1
#define EVENT_DEBUG_STEP 2
#define EVENT_WAITING 4

// Settings
extern volatile bool exiting, is_running, debug_on_start, debug_on_warn;

extern bool turbo_mode;

enum { LOG_CPU, LOG_IO, LOG_FLASH, LOG_INTRPTS, LOG_COUNT, LOG_USB, LOG_GUI, MAX_LOG };
#define LOG_TYPE_TBL "CIFQ#UG"
extern int log_enabled[MAX_LOG];
void logprintf(int type, const char *str, ...);
void emuprintf(const char *format, ...);

void throttle_timer_on();
void throttle_timer_off();
void throttle_timer_wait();

void warn(const char *fmt, ...);
void error(const char *fmt, ...);

// ROM image
extern const char *rom_image;

// GUI callbacks
void gui_do_stuff(bool wait);
void gui_console_printf(const char *, ...);
void gui_console_vprintf(const char *, va_list);
void gui_perror(const char *);
void gui_debugger_entered();

bool emu_start();
void emu_loop(bool reset);
void emu_cleanup(void);

void throttle_interval_event(int index);

#ifdef __cplusplus
}
#endif

#endif
