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

    if(!(desktop->children = List_new()))
    {
        free(desktop);
        return (Desktop *)0;
    }

    desktop->context = context;
    desktop->last_button_state = 0;
    desktop->drag_child = (Window *)0;
    desktop->drag_off_x = 0;
    desktop->drag_off_y = 0;

    desktop->mouse_x = desktop->context->width / 2;
    desktop->mouse_y = desktop->context->height / 2;

    return desktop;
}

Window *Desktop_create_window(
    Desktop *desktop,
    unsigned int x,
    unsigned int y,
    unsigned int width,
    unsigned int height)
{
    Window *window;
    
    if(!(window = Window_new(x, y, width, height, desktop->context)))
    {
        return window;
    }

    if(!List_add(desktop->children, (void *)window))
    {
        free(window);
        return (Window *)0;
    }

    return window;
}

void Desktop_process_mouse(
    Desktop *desktop,
    uint16_t mouse_x,
    uint16_t mouse_y,
    uint8_t mouse_buttons)
{
    int i;
    Window *child;

    desktop->mouse_x = mouse_x;
    desktop->mouse_y = mouse_y;

    if(mouse_buttons)
    {
        if(!desktop->last_button_state)
        {
            for(i = desktop->children->count - 1; i >= 0; --i)
            {
                child = (Window *)List_get_at(desktop->children, i);
                
                if(mouse_x >= child->x && mouse_x < (child->x + child->width) && 
                   mouse_y >= child->y && mouse_y < (child->y + child->height))
                {
                    List_remove_at(desktop->children, i);
                    List_add(desktop->children, (void *)child);
                    
                    desktop->drag_off_x = mouse_x - child->x;
                    desktop->drag_off_y = mouse_y - child->y;
                    desktop->drag_child = child;
                    
                    
                    break;
                }
            }
        }
    }
    else
    {
        desktop->drag_child = (Window *)0;
    }

    if(desktop->drag_child)
    {
        desktop->drag_child->x = mouse_x - desktop->drag_off_x;
        desktop->drag_child->y = mouse_y - desktop->drag_off_y;
    }
    
    Desktop_paint(desktop);

    desktop->last_button_state = mouse_buttons;
}

void Desktop_paint(Desktop *desktop)
{
    /*
     * Desktop painting timing
     */

    struct timespec start_time;
    struct timespec end_time;
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &start_time);


    /*
     * Desktop painting
     */

    unsigned int i;
    unsigned int j;
    Window *current_window;
    Window *clipping_window;
    Rect *temp_rect;
    List *clip_windows;

    temp_rect = Rect_new(0, 0, desktop->context->height - 1, desktop->context->width - 1);
    Context_add_clip_rect(desktop->context, temp_rect);

    for(i = 0; i < desktop->children->count; ++i)
    {
        current_window = (Window *)List_get_at(desktop->children, i);

        temp_rect = Rect_new(current_window->y, current_window->x,
                             current_window->y + current_window->height - 1,
                             current_window->x + current_window->width - 1);

        Context_subtract_clip_rect(desktop->context, temp_rect);
        free(temp_rect);
    }
    
    Context_fill_rect(
        desktop->context, 
        0, 
        0, 
        desktop->context->width, 
        desktop->context->height, 
        0xFFFF9933);

    Context_clear_clip_rects(desktop->context);

    for(i = 0; i < desktop->children->count; ++i)
    {
        current_window = (Window *)List_get_at(desktop->children, i);

        temp_rect = Rect_new(
            current_window->y,
            current_window->x,
            current_window->y + current_window->height - 1,
            current_window->x + current_window->width - 1);

        Context_add_clip_rect(desktop->context, temp_rect);

        clip_windows = Desktop_get_windows_above(desktop, current_window);

        while(clip_windows->count)
        {
            clipping_window = (Window *)List_remove_at(clip_windows, 0);

            if(clipping_window == current_window)
                continue;

            temp_rect = Rect_new(
                clipping_window->y,
                clipping_window->x,
                clipping_window->y + clipping_window->height - 1,
                clipping_window->x + clipping_window->width - 1);

            Context_subtract_clip_rect(desktop->context, temp_rect);
            free(temp_rect);
        }

        Window_paint(current_window);

        free(clip_windows);

        Context_clear_clip_rects(desktop->context);
    }

    Context_fill_rect(
        desktop->context, 
        desktop->mouse_x, 
        desktop->mouse_y, 
        10, 
        10, 
        0xFF000000);


    /*
     * Desktop painting end
     */

    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &end_time);

    long nanoDiff = (end_time.tv_sec - start_time.tv_sec) * (long)1e9 + 
        (end_time.tv_nsec - start_time.tv_nsec);

    printf("Painting Desktop took %dns\n", (int)nanoDiff);
}

List *Desktop_get_windows_above(Desktop *desktop, Window *window)
{
    int i;
    Window *current_window;
    List *return_list;

    if(!(return_list = List_new()))
        return return_list;

    for(i = 0; i < desktop->children->count; ++i)
    {
        if(window == (Window *)List_get_at(desktop->children, i))
            break;
    }

    for(; i < desktop->children->count; ++i)
    {
        current_window = (Window *)List_get_at(desktop->children, i);

        if(current_window->x <= (window->x + window->width - 1) &&
           (current_window->x + current_window->width - 1) >= window->x &&
           current_window->y <= (window->y + window->height - 1) &&
           (window->y + window->height - 1) >= window->y)
        {
            List_add(return_list, current_window);
        }
    }

    return return_list;
}

/* "'(file-name-nondirectory (buffer-file-name))'" ends here */
