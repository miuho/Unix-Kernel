/** @file kern/set_term_color.c
 *
 *  @brief set_term_color syscall implementation
 *
 *  @author Hingon Miu (hmiu@andrew.cmu.edu)
 *  @author An Wu (anwu@andrew.cmu.edu)
 */

#include <syscall_handler.h>

#include <common_include.h>

static char *tag = "set_term_color";

int set_term_color_handler(int color) {
    report_progress(tag, "entry");

    int temp = set_term_color(color);

    report_progress(tag, "exit");

    return temp;
}
