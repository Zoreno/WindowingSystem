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
#define WIN_BORDERCOLOR 0xFF000000

typedef struct Window_struct
{
    uint16_t x;
    uint16_t y;

    uint16_t width;
    uint16_t height;

    Context *context;
} Window;

Window *Window_new(
    unsigned int x, 
    unsigned int y, 
    unsigned int width, 
    unsigned int height,
    Context *context);

void Window_paint(Window *window);

#endif // _WINDOW_H_

/* "'(file-name-nondirectory (buffer-file-name))'" ends here */
