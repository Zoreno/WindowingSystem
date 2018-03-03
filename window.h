/* "'(file-name-nondirectory (buffer-file-name))'" --- 
 * 
 * Filename: "'(file-name-nondirectory (buffer-file-name))'"
 * Description: 
 * Author: Joakim Bertils
 * Maintainer: 
 * Created: "'(current-time-string)'"
 * Version: 
 * Package-Requires: ()
 * Last-Updated: 
 *          
 *     
 * URL: 
 * Doc URL: 
 * Keywords: 
 * Compatibility: 
 * 
 */

/* Commentary: 
 * 
 * 
 * 
 */

/* Change Log:
 * 
 * 
 */

/* This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or (at
 * your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with GNU Emacs.  If not, see <https://www.gnu.org/licenses/>.
 */

/* Code: */

#ifndef _WINDOW_H_
#define _WINDOW_H_

#include "context.h"
#include <stdint.h>

#define WIN_BGCOLOR 0xFFBBBBBB
#define WIN_TITLECOLOR 0xFFBE9270
#define WIN_TITLECOLOR_INACTIVE 0xFF908080
#define WIN_TEXTCOLOR 0xFFFFE0E0
#define WIN_TEXTCOLOR_INACTIVE 0xFFFFBBBB
#define WIN_BORDERCOLOR 0xFF000000

#define WIN_TITLEHEIGHT 31
#define WIN_BORDERWIDTH 3

#define WIN_NODECORATION 0x1
#define WIN_MINIMIZED 0x2

struct Window_struct;

typedef void (*WindowPaintHandler)(struct Window_struct *);
typedef void (*WindowMousedownHandler)(struct Window_struct *, int, int);

typedef struct Window_struct
{
    struct Window_struct *parent;

    int16_t x;
    int16_t y;

    uint16_t width;
    uint16_t height;

    uint16_t flags;

    Context *context;

    List *children;

    uint8_t last_button_state;

    WindowPaintHandler paint_function;

    struct Window_struct *drag_child;
    struct Window_struct *active_child;

    int16_t drag_off_x;
    int16_t drag_off_y;

    WindowMousedownHandler mousedown_function;

    char *title;

    uint32_t index;
} Window;

Window *Window_new(
    unsigned int x, 
    unsigned int y, 
    unsigned int width, 
    unsigned int height,
    uint16_t flags,
    Context *context,
    uint32_t index);

int Window_init(
    Window *window, 
    int16_t x, 
    int16_t y, 
    uint16_t width, 
    uint16_t height,
    uint16_t flags, 
    Context *context,
    uint32_t index);

void Window_paint(
    Window *window,
    List *dirty_regions,
    uint8_t paint_children);

void Window_process_mouse(
    Window *window, 
    uint16_t mouse_x, 
    uint16_t mouse_y, 
    uint8_t mouse_buttons);

List *Window_get_windows_above(
    Window *parent, 
    Window *child);

List *Window_get_windows_below(
    Window *parent,
    Window *child);

Window *Window_create_window(
    Window *window, 
    int16_t x, 
    int16_t y, 
    uint16_t width, 
    uint16_t height, 
    uint16_t flags);

void Window_insert_child(
    Window *window, 
    Window *child);

void Window_raise(
    Window *window, 
    uint8_t do_draw);

void Window_move(
    Window *window, 
    int new_x, 
    int new_y);

void Window_invalidate(
    Window *window, 
    int top, 
    int left, 
    int bottom, 
    int right);

void Window_set_title(
    Window *window, 
    char *new_title);

void Window_minimize(
    Window *window);

void Window_restore(
    Window *window);

void Window_remove(
    Window *window);

#endif // _WINDOW_H_

/* "'(file-name-nondirectory (buffer-file-name))'" ends here */
