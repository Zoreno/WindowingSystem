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

#ifndef _DESKTOP_H_
#define _DESKTOP_H_

#include "list.h"
#include "context.h"
#include "window.h"

typedef struct Desktop_struct
{
    List *children;
    Context *context;

    uint8_t last_button_state;
    
    uint16_t mouse_x;
    uint16_t mouse_y;
    
    Window *drag_child;
    uint16_t drag_off_x;
    uint16_t drag_off_y;
} Desktop;

Desktop *Desktop_new(Context *context);

Window *Desktop_create_window(
    Desktop *desktop, 
    unsigned int x, 
    unsigned int y, 
    unsigned int width, 
    unsigned int height);

void Desktop_process_mouse(
    Desktop *desktop, 
    uint16_t x, 
    uint16_t y, 
    uint8_t mouse_button);

void Desktop_paint(
    Desktop *desktop);

List *Desktop_get_windows_above(Desktop *desktop, Window *window);

#endif // _DESKTOP_H_

/* "'(file-name-nondirectory (buffer-file-name))'" ends here */
