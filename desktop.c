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

#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include "desktop.h"
#include "rect.h"
#include <time.h>
#include "context.h"

Desktop *Desktop_new(Context *context)
{
    Desktop *desktop;

    if(!(desktop = (Desktop *)malloc(sizeof(Desktop))))
    {
        return desktop;
    }

    if(!Window_init(
           (Window *)desktop, 
           0, 
           0, 
           context->width, 
           context->height, 
           WIN_NODECORATION, 
           context))
    {
        free(desktop);
        return (Desktop *)0;
    }

    desktop->window.paint_function = Desktop_paint_handler;

    desktop->window.last_button_state = 0;

    desktop->mouse_x = desktop->window.context->width / 2;
    desktop->mouse_y = desktop->window.context->height / 2;

    return desktop;
}

void Desktop_process_mouse(
    Desktop *desktop, 
    uint16_t mouse_x, 
    uint16_t mouse_y, 
    uint8_t mouse_buttons)
{
    
    struct timespec start_time;
    struct timespec end_time;

    desktop->mouse_x = mouse_x;
    desktop->mouse_y = mouse_y;

    Window_process_mouse((Window *)desktop, mouse_x, mouse_y, mouse_buttons);

    /*
     * Start time measurement
     */

    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &start_time);

    /*
     * Do the painting 
     */


    /*
     * Get the time after painting
     */
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &end_time);

    long nanoDiff = (end_time.tv_sec - start_time.tv_sec) * (long)1e9 + 
        (end_time.tv_nsec - start_time.tv_nsec);

    printf("Painting Desktop took %dns\n", (int)nanoDiff);

    Context_fill_rect(
        desktop->window.context, 
        desktop->mouse_x, 
        desktop->mouse_y, 
        10, 
        10, 
        0xFF000000);
}

void Desktop_paint_handler(Window *desktop_window)
{
    Context_fill_rect(
        desktop_window->context, 
        0, 
        0, 
        desktop_window->context->width,
        desktop_window->context->height, 
        0xFFFF9933);
}

/* "'(file-name-nondirectory (buffer-file-name))'" ends here */
