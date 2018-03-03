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

#ifndef _BUTTON_H_
#define _BUTTON_H_

#include "window.h"

struct Button_struct;

typedef void (*ButtonMousedownHandler)(struct Button_struct *, int, int);

typedef struct Button_struct
{
    Window window;
    uint8_t color_toggle;
    ButtonMousedownHandler on_mousedown;
} Button;

Button *Button_new(int x, int y, int w, int h);
void Button_mousedown_handler(Window *button_window, int x, int y);
void Button_paint(Window *button_window);


#endif // _BUTTON_H_

/* "'(file-name-nondirectory (buffer-file-name))'" ends here */
