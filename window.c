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
    window->active_child = (Window *)0;
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

void Window_update_title(Window *window)
{
    int screen_x;
    int screen_y;

    if(!window->context)
    {
        return;
    }

    if(window->flags & WIN_NODECORATION)
    {
        return;
    }

    Window_apply_bound_clipping(window, 0, (List *)0);

    Window_draw_border(window);

    Context_clear_clip_rects(window->context);
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
    Context_fill_rect(
        window->context, 
        screen_x + 3, 
        screen_y + 3,
        window->width - 6, 
        25, 
        window->parent->active_child == window ? WIN_TITLECOLOR : WIN_TITLECOLOR_INACTIVE);
}

void Window_apply_bound_clipping(Window *window, int in_recursion, List *dirty_regions)
{
    Rect *temp_rect;
    Rect *current_dirty_rect;
    Rect *clone_dirty_rect;
    int screen_x;
    int screen_y;
    int i;
    List *clip_windows;
    Window *clipping_window;

    if(!window->context)
        return;

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
        if(dirty_regions)
        {
            for(i = 0; i < dirty_regions->count; ++i)
            {
                current_dirty_rect = (Rect *)List_get_at(dirty_regions, i);
                clone_dirty_rect = Rect_new(current_dirty_rect->top,
                                            current_dirty_rect->left,
                                            current_dirty_rect->bottom,
                                            current_dirty_rect->right);

                Context_add_clip_rect(window->context, clone_dirty_rect);
            }

            Context_intersect_clip_rect(window->context, temp_rect);
        }
        else
        {
            Context_add_clip_rect(window->context, temp_rect);
        }

        return;
    }

    Window_apply_bound_clipping(window->parent, 1, dirty_regions);

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

void Window_paint(Window *window, List *dirty_regions, uint8_t paint_children)
{
    int i;
    int j;
    int screen_x;
    int screen_y;
    int child_screen_x;
    int child_screen_y;

    Window *current_child;
    Rect *temp_rect;

    if(!window->context)
        return;

    Window_apply_bound_clipping(window, 0, dirty_regions);

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

    if(!paint_children)
    {
        return;
    }

    for(i = 0; i < window->children->count; ++i)
    {
        current_child = (Window *)List_get_at(window->children, i);

        if(dirty_regions)
        {
            for(j = 0; j < dirty_regions->count; ++j)
            {

                temp_rect = (Rect *)List_get_at(dirty_regions, j);
                
                screen_x = Window_screen_x(current_child);
                screen_y = Window_screen_y(current_child);
                
                if(temp_rect->left <= (screen_x + current_child->width - 1) && 
                   temp_rect->right >= screen_x && 
                   temp_rect->top <= (screen_y + current_child->height - 1) &&
                   temp_rect->bottom >= screen_y)
                {
                    break;
                }
            }

            if(j == dirty_regions->count)
            {
                continue;
            }
        }

        Window_paint(current_child, dirty_regions, 1);
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

List *Window_get_windows_below(Window *parent, Window *child)
{
    int i;
    Window *current_window;
    List *return_list;

    if(!(return_list = List_new()))
    {
        return return_list;
    }

    for(i = parent->children->count - 1; i > -1; --i)
    {
        if(child == (Window *)List_get_at(parent->children, i))
        {
            break;
        }
    }

    for(; i > -1; --i)
    {
        current_window = List_get_at(parent->children, i);

        if(current_window->x <= (child->x + child->width - 1) && 
           (current_window->x + current_window->width - 1) >= child->x && 
           current_window->y <= (child->y + child->height - 1) && 
           (current_window->y + current_window->height - 1) >= child->y)
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
            Window_raise(child, 1);

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
        Window_move(
            window->drag_child, 
            mouse_x - window->drag_off_x,
            mouse_y - window->drag_off_y);
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

void Window_update_context(Window *window, Context *context)
{
    int i;

    window->context = context;

    for(i = 0; i < window->children->count; ++i)
    {
        Window_update_context((Window *)List_get_at(window->children, i), context);
    }
}

void Window_insert_child(Window *window, Window *child)
{
    child->parent = window;
    List_add(window->children, child);
    child->parent->active_child = child;

    Window_update_context(child, window->context);
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
    new_window->parent->active_child = new_window;

    return new_window;
}

void Window_raise(Window *window, uint8_t do_draw)
{
    int i;
    Window *parent;
    Window *last_active;

    if(!window->parent)
    {
        return;
    }

    parent = window->parent;

    if(parent->active_child == window)
    {
        return;
    }

    last_active = parent->active_child;

    for(i = 0; i < parent->children->count; ++i)
    {
        if((Window *)List_get_at(parent->children, i) == window)
        {
            break;
        }
    }

    List_remove_at(parent->children, i);
    List_add(parent->children, (void *) window);

    parent->active_child = window;

    if(!do_draw)
    {
        return;
    }

    Window_paint(window, (List *)0, 1);

    Window_update_title(last_active);
}

void Window_move(Window *window, int new_x, int new_y)
{
    int i;
    int old_x = window->x;
    int old_y = window->y;

    Rect new_window_rect;
    List *replacement_list;
    List *dirty_list;
    List *dirty_windows;

    Window_raise(window, 0);

    Window_apply_bound_clipping(window, 0, (List *)0);

    window->x = new_x;
    window->y = new_y;

    new_window_rect.top = Window_screen_y(window);
    new_window_rect.left = Window_screen_x(window);
    new_window_rect.bottom = new_window_rect.top + window->height - 1;
    new_window_rect.right = new_window_rect.left + window->width - 1;

    window->x = old_x;
    window->y = old_y;

    Context_subtract_clip_rect(window->context, &new_window_rect);

    if(!(replacement_list = List_new()))
    {
        Context_clear_clip_rects(window->context);
        return;
    }

    dirty_list = window->context->clip_rects;
    window->context->clip_rects = replacement_list;

    dirty_windows = Window_get_windows_below(window->parent, window);

    window->x = new_x;
    window->y = new_y;

    while(dirty_windows->count)
    {
        Window_paint((Window *)List_remove_at(dirty_windows, 0), dirty_list, 1);
    }

    Window_paint(window->parent, dirty_list, 0);

    while(dirty_list->count)
    {
        free(List_remove_at(dirty_list, 0));
    }

    free(dirty_list);
    free(dirty_windows);

    Window_paint(window, (List *)0, 1);

}

void Window_invalidate(Window *window, int top, int left, int bottom, int right)
{
    List *dirty_regions;
    Rect *dirty_rect;

    int origin_x = Window_screen_x(window);
    int origin_y = Window_screen_y(window);
    
    top += origin_y;
    bottom += origin_y;
    left += origin_x;
    right += origin_x;

    if(!(dirty_regions = List_new()))
    {
        return;
    }

    if(!(dirty_rect = Rect_new(top, left, bottom, right)))
    {
        free(dirty_regions);
        return;
    }
    
    if(!List_add(dirty_regions, dirty_rect))
    {
        free(dirty_regions);
        free(dirty_rect);
        return;
    }

    Window_paint(window, dirty_regions, 0);


    List_remove_at(dirty_regions, 0);
    free(dirty_regions);
    free(dirty_rect);
}

/* "'(file-name-nondirectory (buffer-file-name))'" ends here */
