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
#include "context.h"

void Window_paint_handler(Window *window);
void Window_mousedown_handler(Window *window, int x, int y);


uint8_t pseudo_rand_8()
{
    static uint16_t seed = 0;
    return (uint8_t)(seed = (12657 * seed + 12345) & 256);
}

Window *Window_new(
    unsigned int x, 
    unsigned int y, 
    unsigned int width, 
    unsigned int height,
    uint16_t flags,
    Context *context)
{
    Window *window;
    if(!(window = (Window *)malloc(sizeof(Window))))
    {
        return window;
    }

    if(!Window_init(window, x, y, width, height, flags, context))
    {
        free(window);

        return (Window *)0;
    }

    return window;
}

int Window_init(
    Window *window, 
    int16_t x, 
    int16_t y, 
    uint16_t width, 
    uint16_t height,
    uint16_t flags, 
    Context *context)
{
    if(!(window->children = List_new()))
    {
        return 0;
    }

    window->x = x;
    window->y = y;
    window->width = width;
    window->height = height;
    window->context = context;
    window->flags = flags;
    window->parent = (Window *)0;
    window->last_button_state = 0;
    window->paint_function = Window_paint_handler;
    window->drag_child = (Window *)0;
    window->drag_off_x = 0;
    window->drag_off_y = 0;
    window->mousedown_function = Window_mousedown_handler;

    return 1;
}

int Window_screen_x(Window *window)
{
    if(window->parent)
    {
        return window->x + Window_screen_x(window->parent);
    }

    return window->x;
}

int Window_screen_y(Window *window)
{
    if(window->parent)
    {
        return window->y + Window_screen_y(window->parent);
    }

    return window->y;
}

void Window_draw_border(Window *window)
{
    int screen_x = Window_screen_x(window);
    int screen_y = Window_screen_y(window);

    // Draw the 3 pixel border around the window
    Context_draw_rect(window->context, screen_x, screen_y,
                      window->width, window->height, WIN_BORDERCOLOR);
    Context_draw_rect(window->context, screen_x + 1, screen_y + 1,
                      window->width - 2, window->height - 2, WIN_BORDERCOLOR);
    Context_draw_rect(window->context, screen_x + 2, screen_y + 2,
                      window->width - 4, window->height - 4, WIN_BORDERCOLOR);

    // Draw the 3 pixel border line under the title bar
    Context_horizontal_line(window->context, screen_x + 3, screen_y + 28,
                            window->width - 6, WIN_BORDERCOLOR);

    Context_horizontal_line(window->context, screen_x + 3, screen_y + 29,
                            window->width - 6, WIN_BORDERCOLOR);

    Context_horizontal_line(window->context, screen_x + 3, screen_y + 30,
                            window->width - 6, WIN_BORDERCOLOR);

    // Fill in the color of the title bar.
    Context_fill_rect(window->context, screen_x + 3, screen_y + 3,
                      window->width - 6, 25, WIN_TITLECOLOR);
}

void Window_apply_bound_clipping(Window *window, int in_recursion)
{
    Rect *temp_rect;
    int screen_x;
    int screen_y;
    List *clip_windows;
    Window *clipping_window;

    screen_x = Window_screen_x(window);
    screen_y = Window_screen_y(window);

    if((!(window->flags & WIN_NODECORATION)) && in_recursion)
    {
        screen_x += WIN_BORDERWIDTH;
        screen_y += WIN_TITLEHEIGHT;

        temp_rect = Rect_new(
            screen_y, 
            screen_x, 
            screen_y + window->height - WIN_TITLEHEIGHT - WIN_BORDERWIDTH - 1,
            screen_x + window->width - (2 * WIN_BORDERWIDTH) - 1);
    }
    else   
    {
        temp_rect = Rect_new(screen_y, screen_x, screen_y + window->height - 1,
                             screen_x + window->width - 1);
    }

    if(!window->parent)
    {
        Context_add_clip_rect(window->context, temp_rect);
        return;
    }

    Window_apply_bound_clipping(window->parent, 1);

    Context_intersect_clip_rect(window->context, temp_rect);

    clip_windows = Window_get_windows_above(window->parent, window);
    
    while(clip_windows->count)
    {
        clipping_window = (Window *)List_remove_at(clip_windows, 0);

        if(clipping_window == window)
        {
            continue;
        }

        screen_x = Window_screen_x(clipping_window);
        screen_y = Window_screen_y(clipping_window);

        temp_rect = Rect_new(
            screen_y, 
            screen_x,
            screen_y + clipping_window->height - 1,
            screen_x + clipping_window->width - 1);
        Context_subtract_clip_rect(window->context, temp_rect);
        free(temp_rect);
    }

    free(clip_windows);

}

void Window_paint(Window *window)
{
    int i;
    int screen_x;
    int screen_y;
    int child_screen_x;
    int child_screen_y;

    Window *current_child;
    Rect *temp_rect;

    Window_apply_bound_clipping(window, 0);

    screen_x = Window_screen_x(window);
    screen_y = Window_screen_y(window);

    if(!(window->flags & WIN_NODECORATION))
    {
        Window_draw_border(window);

        screen_x += WIN_BORDERWIDTH;
        screen_y += WIN_TITLEHEIGHT;

        temp_rect = Rect_new(screen_y, screen_x,
                             screen_y + window->height - WIN_TITLEHEIGHT - WIN_BORDERWIDTH - 1,
                             screen_x + window->width - (2*WIN_BORDERWIDTH) - 1);

        Context_intersect_clip_rect(window->context, temp_rect);
    }

    for(i = 0; i < window->children->count; ++i)
    {
        current_child = (Window *)List_get_at(window->children, i);
        
        child_screen_x = Window_screen_x(current_child);
        child_screen_y = Window_screen_y(current_child);

        temp_rect = Rect_new(
            child_screen_y,
            child_screen_x,
            child_screen_y + current_child->height - 1,
            child_screen_x + current_child->width - 1);

        Context_subtract_clip_rect(window->context, temp_rect);
        
        free(temp_rect);
    }

    window->context->translate_x = screen_x;
    window->context->translate_y = screen_y;
    
    window->paint_function(window);
    
    Context_clear_clip_rects(window->context);

    window->context->translate_x = 0;
    window->context->translate_y = 0;

    for(i = 0; i < window->children->count; ++i)
    {
        current_child = (Window *)List_get_at(window->children, i);
        Window_paint(current_child);
    }
}

void Window_paint_handler(Window *window)
{
    Context_fill_rect(
        window->context, 
        0, 
        0, 
        window->width, 
        window->height, 
        WIN_BGCOLOR);
}

List *Window_get_windows_above(Window *parent, Window *window)
{
    int i;
    Window *current_window;
    List *return_list;

    if(!(return_list = List_new()))
        return return_list;

    for(i = 0; i < parent->children->count; ++i)
    {
        if(window == (Window *)List_get_at(parent->children, i))
            break;
    }

    for(; i < parent->children->count; ++i)
    {
        current_window = (Window *)List_get_at(parent->children, i);

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

void Window_process_mouse(
    Window *window, 
    uint16_t mouse_x, 
    uint16_t mouse_y, 
    uint8_t mouse_buttons)
{
    int i;
    int inner_x1;
    int inner_y1;
    int inner_x2;
    int inner_y2;
    
    Window *child;
    
    for(i = window->children->count - 1; i >= 0; --i)
    {
        child = (Window *)List_get_at(window->children, i);

        if(!(mouse_x >= child->x && mouse_x < (child->x + child->width) &&
           mouse_y >= child->y && mouse_y < (child->y + child->height)))
        {
            continue;
        }

        if(mouse_buttons && !window->last_button_state)
        {
            List_remove_at(window->children, i);
            List_add(window->children, (void *)child);

            if(!(child->flags & WIN_NODECORATION) &&
               mouse_y >= child->y && mouse_y < (child->y + 31))
            {
                window->drag_off_x = mouse_x - child->x;
                window->drag_off_y = mouse_y - child->y;
                window->drag_child = child;

                break;
            }
        }

        Window_process_mouse(child, mouse_x - child->x, mouse_y - child->y, mouse_buttons);
        break;
    }

    if(!mouse_buttons)
    {
        window->drag_child = (Window *)0;
    }

    if(window->drag_child)
    {
        window->drag_child->x = mouse_x - window->drag_off_x;
        window->drag_child->y = mouse_y - window->drag_off_y;
    }

    if(window->mousedown_function && mouse_buttons && !window->last_button_state)
    {
        window->mousedown_function(window, mouse_x, mouse_y);
    }

    window->last_button_state = mouse_buttons;
}

void Window_mousedown_handler(Window *window, int x, int y)
{
    return;
}

void Window_insert_child(Window *window, Window *child)
{
    child->parent = window;
    child->context = window->context;
    List_add(window->children, child);
}

Window *Window_create_window(Window *window, int16_t x, int16_t y,
                            uint16_t width, uint16_t height, uint16_t flags)
{
    Window *new_window;

    if(!(new_window = Window_new(x, y, width, height, flags, window->context)))
    {
        return new_window;
    }

    if(!List_add(window->children, (void *)new_window))
    {
        free(new_window);
        return (Window *)0;
    }

    new_window->parent = window;

    return new_window;
}

/* "'(file-name-nondirectory (buffer-file-name))'" ends here */
