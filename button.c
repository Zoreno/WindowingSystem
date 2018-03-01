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

#include "button.h"

Button *Button_new(int x, int y, int w, int h)
{
    Button *button;
    if(!(button = (Button *)malloc(sizeof(Button))))
    {
        return button;
    }

    if(!Window_init((Window *) button, x, y, w, h, WIN_NODECORATION, (Context *) 0))
    {
        free(button);
        return (Button *)0;
    }

    button->window.paint_function = Button_paint;
    button->window.mousedown_function = Button_mousedown_handler;

    button->color_toggle = 0;

    return button;
}

void Button_paint(Window *button_window)
{
    Button *button = (Button *)button_window;

    uint32_t border_color;

    if(button->color_toggle)
    {
        border_color = WIN_TITLECOLOR;
    }
    else
    {
        border_color = WIN_BGCOLOR - 0x101010;
    }

    Context_fill_rect(button_window->context, 1, 1, button_window->width - 1, 
                      button_window->height - 1, WIN_BGCOLOR);
    Context_draw_rect(button_window->context, 0, 0, button_window->width, 
                      button_window->height, 0xFF000000);
    Context_draw_rect(button_window->context, 3, 3, button_window->width - 6,
                      button_window->height - 6, border_color);
    Context_draw_rect(button_window->context, 4, 4, button_window->width - 8,
                      button_window->height - 8, border_color);
}

void Button_mousedown_handler(Window *button_window, int x, int y)
{
    Button *button = (Button *)button_window;

    button->color_toggle = !button->color_toggle;

    Window_invalidate((Window *)button, 0, 0,
                      button->window.height - 1, button->window.width - 1);
}
/* "'(file-name-nondirectory (buffer-file-name))'" ends here */
