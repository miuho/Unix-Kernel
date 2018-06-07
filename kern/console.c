/** @file console.c
 *
 *  @brief Write characters to the console.
 *
 *  @author HingOn Miu (hmiu)
 */

#include <console.h>
#include <x86/video_defines.h>
#include <x86/asm.h>
#include <simics.h>
#include <limits.h>
#include <stdio.h>
#include <cond.h>
#include <malloc.h>
#include <syscall.h>
#include <reporter.h>

/* a random number to make the index goes out of the console */
#define RAN_NUM 0xbeef
/* console screen dimension */
#define CONSOLE_SIZE (CONSOLE_WIDTH * CONSOLE_HEIGHT)
/* the most significant bits of console index that hides the cursor */
#define HIDE_OFFSET_MSB (((CONSOLE_SIZE + RAN_NUM) >> CHAR_BIT) & UCHAR_MAX)
/* the least significant bits of console index that hides the cursor */
#define HIDE_OFFSET_LSB ((CONSOLE_SIZE + RAN_NUM) & UCHAR_MAX)
/* the default color on the console */
#define DEFAULT_COLOR ((!BLINK) | FGND_WHITE | BGND_BLACK)

circ_buf_t cons_buf;

queue cons_cond_queue;

mutex_t outb_mp;

static char *tag = "console";

/** @brief Records whether the user decides to hide the cursor.
 **/
int cursor_is_visible = 1;
/** @brief Records cursor's actual console row location.
 **/
int cursor_row = 0;
/** @brief Records cursor's actual console col location.
 **/
int cursor_col = 0;
/** @brief Records the default color for writing characters to the console.
 **/
int term_color = (!BLINK) | FGND_WHITE | BGND_BLACK;

/** @brief The most significant bits of cursor's actual console location.
 *
 *  @return 8 bits msb of cursor's location.
 **/
uint8_t get_offset_msb();

/** @brief The least significant bits of cursor's actual console location.
 *
 *  @return 8 bits lsb of cursor's location.
 **/
uint8_t get_offset_lsb();

/** @brief Get the color of the location on console.
 *
 *   @param row The row index on console
 *   @param col The column index on console
 *
 *   @return The color of the specific location on console.
 **/
char get_color(int row, int col);

/** @brief Scroll up the console screen if needed by shifting every character
 *         up by one row.
 *
 *   @return Void.
 **/
void console_scroll_up();

/** @brief Find the last non-space character at the specific row on console.
 *
 *   @param row The row index on console
 *
 *   @return The column index on the console.
 **/
int find_last_char(int row);


/** @brief Set the cursor location at row and col index.
 *
 *   @param row The row index on console
 *   @param col The col index on console
 *
 *   @return 0 if successful, -1 if fail.
 **/
int set_cursor(int row, int col);


int cons_init() {

    if (cb_init(&cons_buf, PAGE_SIZE) != 0) {
        report_error(tag, "cons_init: fail to initialize circular buffer");
        return -1;
    }


    if ((cons_cond_queue = queue_new()) == NULL) {
        report_error(tag, "cons_init: fail to allocate cons_cond_queue");
        return -1;
    }

    if (mutex_init(&outb_mp) != 0) {
        report_error(tag, "cons_init: fail to initialize mutex");
        return -1;
    }

    clear_console();

    return 0;
}


int putbyte(char ch)
{
    switch (ch) {
        case '\n':
            if (cursor_row + 1 == CONSOLE_HEIGHT) {
                /* bottom row of the console, scroll up one line */
                console_scroll_up();
                set_cursor(CONSOLE_HEIGHT - 1, 0);
            }
            else {
                set_cursor(cursor_row + 1, 0);
            }
            break;

        case '\r':
            /* overwrites from the beginning of the line */
            set_cursor(cursor_row, 0);
            break;

        case '\b':
            if (cursor_col == 0) {
                if (cursor_row == 0) {
                    /* do nothing if at the right top cornor of the console */
                    break;
                }
                else {
                    /* delete character at the beginning of the line */
                    set_cursor(cursor_row - 1, find_last_char(cursor_row - 1));
                }
            }
            else {
                set_cursor(cursor_row, cursor_col - 1);
            }
            draw_char(cursor_row, cursor_col, ' ', term_color);
            break;

        default:
            draw_char(cursor_row, cursor_col, ch, term_color);
            if (cursor_col + 1 == CONSOLE_WIDTH) {
                if (cursor_row + 1 == CONSOLE_HEIGHT) {
                    /* the last row of the console is full */
                    console_scroll_up();
                    set_cursor(CONSOLE_HEIGHT - 1, 0);
                }
                else {
                    /* write to the beginning of the next row */
                    set_cursor(cursor_row + 1, 0);
                }
            }
            else {
                set_cursor(cursor_row, cursor_col + 1);
            }
    }

    return (int)ch;
}

void putbytes(const char *s, int len)
{
  if (s == NULL || len <= 0) {
    return;
  }

  else {
    int i;
    for (i = 0; i < len; i++) {
      putbyte(s[i]);
    }
    return;
  }
}

void console_scroll_up()
{
  int i, j;
  int ch, color;
  for (i = 1; i < CONSOLE_HEIGHT; i++) {
    for (j = 0; j < CONSOLE_WIDTH; j++) {
      /* copy each row to its previous row */
      ch = (int)get_char(i, j);
      color = (int)get_color(i, j);
      draw_char(i - 1, j, ch, color);
    }
  }

  for(j = 0; j < CONSOLE_WIDTH; j++) {
    /* clear the last row */
    draw_char(CONSOLE_HEIGHT - 1, j, ' ', term_color);
  }
  return;
}

int set_term_color(int color)
{
  /* check if the color actually contains one byte of information */
  if (color < 0 || color > UCHAR_MAX) {
    return -1;
  }

  else {
    term_color = color;
    return 0;
  }
}

void get_term_color(int *color)
{
  /* make sure the memory location is not NULL */
  if (color == NULL) {
    return;
  }

  else {
    *color = term_color;
    return;
  }
}

int set_cursor(int row, int col)
{
  /* check if the row and col index are within bound */
  if (row < 0 || row >= CONSOLE_HEIGHT ||
      col < 0 || col >= CONSOLE_WIDTH) {
    return -1;
  }

  else if (!cursor_is_visible) {
    /* if cursor is invisible, remember the new location of the cursor, but */
    /* don't show the cursor immediately, wait for show_cursor() to do that */
    cursor_row = row;
    cursor_col = col;
    return 0;
  }

  else {
    cursor_row = row;
    cursor_col = col;

    mutex_lock(&outb_mp);

    /* because the data register is 1 byte and the cursor offset is 16-bit, */
    /* the offset is splitted into most and least significant 8 bits */
    outb(CRTC_IDX_REG, CRTC_CURSOR_MSB_IDX);
    outb(CRTC_DATA_REG, get_offset_msb());
    outb(CRTC_IDX_REG, CRTC_CURSOR_LSB_IDX);
    outb(CRTC_DATA_REG, get_offset_lsb());

    mutex_unlock(&outb_mp);
    return 0;
  }
}

void get_cursor(int *row, int *col)
{
  /* make sure the memory locations are not NULL */
  if (row == NULL || col == NULL) {
    return;
  }

  else {
    /* it fetches the true location of the cursor, no matter the cursor is */
    /* visible or not */
    *row = cursor_row;
    *col = cursor_col;
    return;
  }
}

void hide_cursor()
{
  if (!cursor_is_visible) {
    /* do nothing if the cursor is invisible already */
    return;
  }

  else {
    mutex_lock(&outb_mp);

    /* because the data register is 1 byte and the cursor offset is 16-bit, */
    /* the offset is splitted into most and least significant 8 bits */
    outb(CRTC_IDX_REG, CRTC_CURSOR_MSB_IDX);
    outb(CRTC_DATA_REG, (uint8_t)HIDE_OFFSET_MSB);
    outb(CRTC_IDX_REG, CRTC_CURSOR_LSB_IDX);
    outb(CRTC_DATA_REG, (uint8_t)HIDE_OFFSET_LSB);

    mutex_unlock(&outb_mp);

    cursor_is_visible = 0;
    return;
  }
}

void show_cursor()
{
  if (cursor_is_visible) {
    /* do nothing if the cursor is visible already */
    return;
  }

  else {
    mutex_lock(&outb_mp);

    /* because the data register is 1 byte and the cursor offset is 16-bit, */
    /* the offset is splitted into most and least significant 8 bits */
    outb(CRTC_IDX_REG, CRTC_CURSOR_MSB_IDX);
    outb(CRTC_DATA_REG, get_offset_msb());
    outb(CRTC_IDX_REG, CRTC_CURSOR_LSB_IDX);
    outb(CRTC_DATA_REG, get_offset_lsb());

    mutex_unlock(&outb_mp);

    cursor_is_visible = 1;
    return;
  }
}

uint8_t get_offset_msb()
{
  return ((cursor_row * CONSOLE_WIDTH + cursor_col) >> CHAR_BIT) & UCHAR_MAX;
}

uint8_t get_offset_lsb()
{
  return (cursor_row * CONSOLE_WIDTH + cursor_col) & UCHAR_MAX;
}

void clear_console()
{
  int i, j;
  for (i = 0; i < CONSOLE_HEIGHT; i++) {
    for (j = 0; j < CONSOLE_WIDTH; j++) {
      /* clear each element in the console */
      draw_char(i, j, ' ', DEFAULT_COLOR);
    }
  }

  /* set the cursor to the console's top left corner */
  set_cursor(0, 0);
  return;
}

int find_last_char(int row)
{
  int j;
  char ch;
  for (j = CONSOLE_WIDTH - 1; j >= 0; j--) {
    /* check each character from the end of the line */
    ch = get_char(row, j);
    if (ch != ' ') {
      return j;
    }
  }
  /* the whole line is empty, filled with space characters */
  return 0;
}

/** @brief Draw the character at row and col index.
 *
 *   @param row The row index on console
 *   @param col The col index on console
 *   @param ch The character to be drawn.
 *   @param color The color of the character.
 *
 *   @return Void.
 **/
void draw_char(int row, int col, int ch, int color)
{
  /* check if the row and col index are within bound */
  if (row < 0 || row >= CONSOLE_HEIGHT ||
      col < 0 || col >= CONSOLE_WIDTH) {
    return;
  }

  /* check if the color actually contains one byte of information */
  else if (color < 0 || color > UCHAR_MAX) {
    return;
  }

  else {
    /* write the byte pair to video memory */
    *(char *)(CONSOLE_MEM_BASE + 2*(row * CONSOLE_WIDTH + col)) = ch;
    *(char *)(CONSOLE_MEM_BASE + 2*(row * CONSOLE_WIDTH + col) + 1) = color;
    return;
  }
}

char get_color(int row, int col)
{
  /* check if the row and col index are within bound */
  if (row < 0 || row >= CONSOLE_HEIGHT ||
      col < 0 || col >= CONSOLE_WIDTH) {
    /* returns the default color if inputs are invalid */
    return (char)((!BLINK) | FGND_WHITE | BGND_BLACK);
  }

  else {
    return *(char *)(CONSOLE_MEM_BASE + 2*(row * CONSOLE_WIDTH + col) + 1);
  }
}

/** @brief Get the character at row and col index.
 *
 *   @param row The row index on console
 *   @param col The col index on console
 *
 *   @return The character on the console.
 **/
char get_char(int row, int col)
{
  /* check if the row and col index are within bound */
  if (row < 0 || row >= CONSOLE_HEIGHT ||
      col < 0 || col >= CONSOLE_WIDTH) {
    return (char)'\0';
  }

  else {
    return *(char *)(CONSOLE_MEM_BASE + 2*(row * CONSOLE_WIDTH + col));
  }
}
