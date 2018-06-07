/** @file kern/reporter.c
 *
 *  @brief out debugging interface: report based on VERBOSE_LEVEL
 *
 *  @author Hingon Miu (hmiu@andrew.cmu.edu)
 *  @author An Wu (anwu@andrew.cmu.edu)
 */

#include <reporter.h>

#include <simics.h>
#include <stdarg.h>
#include <x86/asm.h>
#include <stdio.h>
#include <loader.h>
#include <if_flag.h>

char progress_buf[256];
char warning_buf[256];
char error_buf[256];
char misc_buf[256];


void report_misc(const char *tag, const char *fmt, ...) {
    if (VERBOSE_LEVEL > 3) {
        
        int if_was_set = if_disable();

        va_list args;
        va_start(args, fmt);
        vsprintf(misc_buf, fmt, args);
        va_end(args);
        tid_t tid = (running_ktcb) ? (running_ktcb->tcb->tid) : 0;

        lprintf("MISC Thread %d %s: %s", tid, tag, misc_buf);

        if_recover(if_was_set);
    }
}
    
void report_progress(const char *tag, const char *fmt, ...) {
    if (VERBOSE_LEVEL > 2) {
        
        int if_was_set = if_disable();

        va_list args;
        va_start(args, fmt);
        vsprintf(progress_buf, fmt, args);
        va_end(args);
        tid_t tid = (running_ktcb) ? (running_ktcb->tcb->tid) : 0;

        lprintf("PROGRESS Thread %d %s: %s", tid, tag, progress_buf);

        if_recover(if_was_set);
    }
        
}

void report_warning(const char *tag, const char *fmt, ...) {
    if (VERBOSE_LEVEL > 1) {
        
        int if_was_set = if_disable();

        va_list args;
        va_start(args, fmt);
        vsprintf(warning_buf, fmt, args);
        va_end(args);
        tid_t tid = (running_ktcb) ? (running_ktcb->tcb->tid) : 0;

        lprintf("!WARNING Thread %d %s: %s", tid, tag, warning_buf);

        if_recover(if_was_set);
    }
}

void report_error(const char *tag, const char *fmt, ...) {
    if (VERBOSE_LEVEL > 0) {
        
        int if_was_set = if_disable();

        va_list args;
        va_start(args, fmt);
        vsprintf(error_buf, fmt, args);
        va_end(args);
        tid_t tid = (running_ktcb) ? (running_ktcb->tcb->tid) : 0;

        lprintf("!!ERROR Thread %d %s: %s", tid, tag, error_buf);

        if_recover(if_was_set);
    }
}
