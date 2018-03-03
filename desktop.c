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
#include <string.h>
#include "desktop.h"
#include "rect.h"
#include <time.h>
#include "context.h"

#define cX 0xFF000000
#define cO 0xFFFFFFFF
#define c_ 0x00000000

unsigned int mouse_img[MOUSE_BUFSZ] = {  
    cX, c_, c_, c_, c_, c_, c_, c_, c_, c_, c_,
    cX, cX, c_, c_, c_, c_, c_, c_, c_, c_, c_,
    cX, cO, cX, c_, c_, c_, c_, c_, c_, c_, c_,
    cX, cO, cO, cX, c_, c_, c_, c_, c_, c_, c_,
    cX, cO, cO, cO, cX, c_, c_ ,c_, c_, c_, c_,
    cX, cO, cO, cO, cO, cX, c_, c_, c_, c_, c_,
    cX, cO, cO, cO, cO, cO, cX, c_, c_, c_, c_,
    cX, cO, cO, cO, cO, cO, cO, cX, c_, c_, c_,
    cX, cO, cO, cO, cO, cO, cO, cO, cX, c_, c_,
    cX, cO, cO, cO, cO, cO, cO, cO, cO, cX, c_,
    cX, cO, cO, cO, cO, cO, cO, cO, cO, cO, cX,
    cX, cO, cO, cX, cO, cO, cO, cX, cX, cX, cX,
    cX, cO, cX, c_, cX, cO, cO, cX, c_, c_, c_,
    cX, cX, c_, c_, cX, cO, cO, cX, c_, c_, c_,
    cX, c_, c_, c_, c_, cX, cO, cO, cX, c_, c_,
    c_, c_, c_, c_, c_, cX, cO, cO, cX, c_, c_,
    c_, c_, c_, c_, c_, c_, cX, cO, cX, c_, c_,
    c_, c_, c_, c_, c_, c_, c_, cX, cX, c_, c_ 
};

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
           context,
           0))
    {
        free(desktop);
        return (Desktop *)0;
    }

    desktop->window.parent = (Window *)0;

    desktop->window.paint_function = Desktop_paint_handler;
    desktop->window.key_function = (WindowKeyHandler)0;

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

    int i;
    int x;
    int y;

    Window *child;
    Window *desktop_window = (Window *)desktop;
    List *dirty_list;
    Rect *mouse_rect;

    List *pending_remove;


    /*
     * Start time measurement
     */

    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &start_time);

    /*
     * Do the mouse handling/painting
     */
    Window_process_mouse((Window *)desktop, mouse_x, mouse_y, mouse_buttons);

    if(pending_remove = List_new())
    {
        for(i = 0; i < desktop_window->children->count; ++i)
        {
            child = (Window *)List_get_at(desktop_window->children, i);
            
            if(child->flags & WIN_SHOULD_CLOSE)
            {
                List_add(pending_remove, child);
            }
        }
        
        while(pending_remove->count)
        {
            child = (Window *)List_remove_at(pending_remove, 0);
            
            Desktop_remove_window(desktop_window, child);
        }
        
        free(pending_remove);
    }

    if(!(dirty_list = List_new()))
    {
        return;
    }

    if(!(mouse_rect = Rect_new(desktop->mouse_y, desktop->mouse_x,
                               desktop->mouse_y + MOUSE_HEIGHT - 1,
                               desktop->mouse_x + MOUSE_WIDTH - 1)))
    {
        free(dirty_list);
        return;
    }

    List_add(dirty_list, mouse_rect);

    Window_paint(desktop_window, dirty_list, 1);

    List_remove_at(dirty_list, 0);
    free(dirty_list);
    free(mouse_rect);

    desktop->mouse_x = mouse_x;
    desktop->mouse_y = mouse_y;

    for(y = 0; y < MOUSE_HEIGHT; ++y)
    {
        if((y + mouse_y) >= desktop->window.context->height)
        {
            break;
        }

        for(x = 0; x < MOUSE_WIDTH; ++x)
        {
            if((x + mouse_x) >= desktop->window.context->width)
            {
                break;
            }

            if(mouse_img[y * MOUSE_WIDTH + x] & 0xFF000000)
            {
                desktop->window.context->buffer[
                    (y + mouse_y) * desktop->window.context->width + (x + mouse_x)] = 
                    mouse_img[y*MOUSE_WIDTH + x];
            }
        }
    }


    /*
     * Get the time after updating
     */
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &end_time);

    long nanoDiff = (end_time.tv_sec - start_time.tv_sec) * (long)1e9 + 
        (end_time.tv_nsec - start_time.tv_nsec);

    printf("Painting Desktop took %dns\n", (int)nanoDiff);
}

#define START_BAR_HEIGHT 32
#define START_BAR_ICON_WIDTH 64

void Desktop_invalidate_start_bar(Window *desktop_window)
{
    Window_invalidate(
        desktop_window, 
        0,
        desktop_window->height - START_BAR_HEIGHT,
        desktop_window->width,
        desktop_window->height);
}

void Desktop_draw_start_bar(Window *desktop_window)
{
    int i;
    Window *child;
    int active_child;
    
    Context_fill_rect(
        desktop_window->context,
        0,
        desktop_window->height - START_BAR_HEIGHT,
        desktop_window->width,
        START_BAR_HEIGHT,
        0xFF101010);

    for(i = 0; i < desktop_window->children->count; ++i)
    {
        child = (Window *)List_get_at(desktop_window->children, i);

        active_child = desktop_window->active_child && (desktop_window->active_child == child);

        Context_fill_rect(
            desktop_window->context,
            32 + (START_BAR_ICON_WIDTH + 10)*i,
            desktop_window->height - 32 + 6,
            START_BAR_ICON_WIDTH,
            20,
            active_child ? 0xFFFFFFFF : 0xFF4F4F4F);

        char *string[32];

        sprintf(string, "%d", child->index);
        
        Context_draw_text(
            desktop_window->context, 
            string, 
            40 + (START_BAR_ICON_WIDTH + 10)*i,
            desktop_window->height - 32 + 6, 0xFFFF0000);
    }
}

void Desktop_paint_handler(Window *desktop_window)
{
    // TODO: Draw bitmap when possible
    Context_fill_rect(
        desktop_window->context, 
        0, 
        0, 
        desktop_window->context->width,
        desktop_window->context->height, 
        0xFFFF9933);

    Desktop_draw_start_bar(desktop_window);  
}

Window *Desktop_create_window(
    Window *desktop_window, 
    int16_t x, 
    int16_t y, 
    uint16_t width, 
    uint16_t height, 
    uint16_t flags, 
    char *title)
{
    Window *new_window = Window_create_window(desktop_window, x, y, width, height, flags);

    if(!new_window)
    {
        return new_window;
    }

    Window_set_title(new_window, title);

    Desktop_invalidate_start_bar(desktop_window);

    Window_paint(desktop_window, (List *)0, 1);

    return new_window;
}

void Desktop_remove_window(
    Window *desktop_window, 
    Window *window)
{

    int window_index = window->index;
    int i;

    Window *child;

    for(i = 0; i < desktop_window->children->count; ++i)
    {
        child = (Window *)List_get_at(desktop_window->children, i);

        if(child->index > window_index)
        {
            --child->index;
        }
    }

    Window_remove(window);

    desktop_window->active_child = (Window *)0;

    Desktop_invalidate_start_bar(desktop_window);

    Window_paint(desktop_window, (List *)0, 1);
}

/* "'(file-name-nondirectory (buffer-file-name))'" ends here */
