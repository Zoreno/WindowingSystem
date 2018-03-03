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

#include "textbox.h"
#include <string.h>

TextBox *TextBox_new(int x, int y, int width, int height)
{
    TextBox *text_box;
    
    if(!(text_box = (TextBox *)malloc(sizeof(TextBox))))
    {
        return text_box;
    }

    if(!Window_init((Window *)text_box, x, y, width, height, WIN_NODECORATION, (Context *)0, 0))
    {
        free(text_box);
        return (TextBox *)0;
    }

    text_box->window.paint_function = TextBox_paint;

    return text_box;
}

void TextBox_paint(Window *text_box_window)
{
    int title_len;

    Context_fill_rect(
        text_box_window->context, 
        1,
        1,
        text_box_window->width - 2,
        text_box_window->height - 2, 
        0xFFFFFFFF);

   Context_draw_rect(
        text_box_window->context, 
        0,
        0,
        text_box_window->width,
        text_box_window->height, 
        0xFF000000);

   if(!text_box_window->title)
   {
       return;
   }
   
   title_len = strlen(text_box_window->title) * 8;

   Context_draw_text(
       text_box_window->context,
       text_box_window->title,
       text_box_window->width - title_len - 6,
       (text_box_window->height / 2) - 6,
       0xFF000000);
     
}

/* "'(file-name-nondirectory (buffer-file-name))'" ends here */
