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

#include "window.h"

Window *Window_new(
    unsigned int x, 
    unsigned int y, 
    unsigned int width, 
    unsigned int height,
    Context *context)
{
    Window *window;
    if(!(window = (Window *)malloc(sizeof(Window))))
    {
        return window;
    }

    window->x = x;
    window->y = y;
    window->width = width;
    window->height = height;

    window->context = context;

    return window;
}

void Window_paint(Window *window)
{

    Context_draw_rect(
        window->context,
        window->x,
        window->y,
        window->width,
        window->height,
        WIN_BORDERCOLOR);
    
    Context_draw_rect(
        window->context,
        window->x + 1,
        window->y + 1,
        window->width - 2,
        window->height - 2,
        WIN_BORDERCOLOR);

    Context_draw_rect(
        window->context,
        window->x + 2,
        window->y + 2,
        window->width - 4,
        window->height - 4,
        WIN_BORDERCOLOR);

    Context_horizontal_line(
        window->context, 
        window->x + 3, 
        window->y + 28, 
        window->width - 6, 
        WIN_BORDERCOLOR);

    Context_horizontal_line(
        window->context, 
        window->x + 3, 
        window->y + 29, 
        window->width - 6, 
        WIN_BORDERCOLOR);

    Context_horizontal_line(
        window->context, 
        window->x + 3, 
        window->y + 30, 
        window->width - 6, 
        WIN_BORDERCOLOR);

    Context_fill_rect(
        window->context, 
        window->x + 3, 
        window->y + 3,
        window->width - 6, 
        25, 
        WIN_TITLECOLOR);

    Context_fill_rect(
        window->context, 
        window->x + 3, 
        window->y + 31,
        window->width - 6, 
        window->height - 34, 
        WIN_BGCOLOR);

    
}

/* "'(file-name-nondirectory (buffer-file-name))'" ends here */
