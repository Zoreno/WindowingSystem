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

#include "window.h"
#include "context.h"
#include "desktop.h"
#include "button.h"

void Window_paint_handler(Window *window);
void Window_mousedown_handler(Window *window, int x, int y);
void Window_key_handler(Window *window, int key, int mods, int action);
void Window_tick_handler(Window *window, int ticks);

/*
  A quick way to generate a random 8 bit number
 */
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
    Context *context,
    uint32_t index)
{
    Window *window;

    // Allocate memory for the new window struct
    if(!(window = (Window *)malloc(sizeof(Window))))
    {
        return window;
    }
    
    // Initialize the window struct
    if(!Window_init(window, x, y, width, height, flags, context, index))
    {
        free(window);

        return (Window *)0;
    }


    // Return the window.
    return window;
}

int Window_init(
    Window *window, 
    int16_t x, 
    int16_t y, 
    uint16_t width, 
    uint16_t height,
    uint16_t flags, 
    Context *context,
    uint32_t index)
{
    // Initialize the list of children and return if it does not work
    if(!(window->children = List_new()))
    {
        return 0;
    }

    // Initiate parent to zero. This will be linked later if the
    // window has a parent.
    window->parent = (Window *)0;

    // Initiate state variables
    window->x = x;
    window->y = y;
    window->width = width;
    window->height = height;

    if(flags & WIN_NODECORATION)
    {
        window->inner_width = width;
        window->inner_height = height;
    }
    else
    {
        window->inner_width = width - 2*WIN_BORDERWIDTH;
        window->inner_height = height - WIN_TITLEHEIGHT - 2*WIN_BORDERWIDTH;
    }
    
    window->flags = flags;
    window->context = context;
    window->last_button_state = 0;
    window->last_tick = 0;
    
    window->title = (char *)0;
    window->index = index;

    window->drag_child = (Window *)0;
    window->active_child = (Window *)0;
    
    window->dragging = 0;
    window->resizing = 0;

    window->drag_off_x = 0;
    window->drag_off_y = 0;

    // Initiate the standard window handling functions.
    window->paint_function = Window_paint_handler;
    window->mousedown_function = Window_mousedown_handler;    
    window->key_function = Window_key_handler;
    window->tick_function = Window_tick_handler;

    // Initiate settings
    window->min_width = WIN_DEFAULT_MIN_WIDTH;
    window->min_height = WIN_DEFAULT_MIN_HEIGHT;
    window->max_width = WIN_DEFAULT_MAX_WIDTH;
    window->max_height = WIN_DEFAULT_MAX_HEIGHT;

    // TODO: Use a MAX/MIN/CLAMP macro
    // Check if state is valid given the settings
    window->width = window->width > window->min_width ? window->width : window->min_width;
    window->height = window->height > window->min_height ? window->height : window->min_height;

    window->width = window->width < window->max_width ? window->width : window->max_width;
    window->height = window->height < window->max_height ? window->height : window->max_height;

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

    // Make sure that we have a context.
    // A redraw may be triggered before we have initialized the context.
    if(!window->context)
    {
        return;
    }
    
    // If the window is minimized, the window title should not be redrawn
    if(window->flags & WIN_MINIMIZED)
    {
        return;
    }

    // If the window does not have a title, it should not be rendered.
    if(window->flags & WIN_NODECORATION)
    {
        return;
    }

    Window_apply_bound_clipping(window, 0, (List *)0);

    // Redraw the border
    Window_draw_border(window);

    Context_clear_clip_rects(window->context);
}


void Window_draw_border(Window *window)
{
    // Make sure that the position is relative to the full window.
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

    // Render the window title.
    Context_draw_text(
        window->context,
        window->title,
        screen_x + 10,
        screen_y + 10,
        window->parent->active_child == window ? WIN_TEXTCOLOR : WIN_TEXTCOLOR_INACTIVE);

    // Render the 'close window' button.
    Context_fill_rect(
        window->context,
        screen_x + window->width - 24,
        screen_y + 8,
        16,
        16,
        0xFFFF0000);

    // Render the text on the 'close window' button.
    Context_draw_text(
        window->context,
        "x",
        screen_x + window->width - 20,
        screen_y + 8,
        0xFF000000);

}

void Window_apply_bound_clipping(
    Window *window, 
    int in_recursion, 
    List *dirty_regions)
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

    // Get the position relative to the screen.
    screen_x = Window_screen_x(window);
    screen_y = Window_screen_y(window);

    // Get the rect which represents the window rendering area.
    // We must account for the header bar.
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


    // Apply bound clipping for the parent
    Window_apply_bound_clipping(window->parent, 1, dirty_regions);

    Context_intersect_clip_rect(window->context, temp_rect);

    // Get the clip of all the windows above this window.
    clip_windows = Window_get_windows_above(window->parent, window);
    
    // Remove all clip rectangles that intersects the current window rectangle.
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

void Window_paint(
    Window *window, 
    List *dirty_regions, 
    uint8_t paint_children)
{
    int i;
    int j;
    uint32_t current_index;
    int screen_x;
    int screen_y;
    int child_screen_x;
    int child_screen_y;

    Window *current_child;
    Rect *temp_rect;
    List *draw_windows;

    if(!window->context)
    {
        return;
    }

    if(window->flags & WIN_MINIMIZED)
    {
        return;
    }

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

        if((current_child->flags & WIN_MINIMIZED) ||
           (current_child->flags & WIN_FLOATING))
        {
            continue;
        }
        
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

    if(!(draw_windows = List_new()))
    {
       
        return;
    }

    /*
     * Do the first rendering pass with all non-floating windows
     */
    
    // Construct a new list with all windows in drawing order
    for(current_index = 0; current_index < window->children->count; ++current_index)
    {
        for(i = 0; i < window->children->count; ++i)
        {
            current_child = (Window *)List_get_at(window->children, i);

            if(current_child->index == current_index && 
               !(current_child->flags & WIN_FLOATING))
            {
                List_add(draw_windows, current_child);
                continue;
            }    
        }
    }
    
    // Draw all windows in the drawing list
    for(i = 0; i < draw_windows->count; ++i)
    {
        current_child = (Window *)List_get_at(draw_windows, i);
        
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

    while(draw_windows->count)
    {
        List_remove_at(draw_windows, 0);
    }

    /*
     * Do the second rendering pass with the floating windows
     */

    for(i = 0; i < window->children->count; ++i)
    {
        current_child = (Window *)List_get_at(window->children, i);
        
        if(!(current_child->flags & WIN_FLOATING))
        {
            continue;
        }

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
   
    free(draw_windows);
}

void Window_paint_handler(Window *window)
{

    // Just fill in the background
    Context_fill_rect(
        window->context, 
        0, 
        0, 
        window->width, 
        window->height, 
        WIN_BGCOLOR);
}

List *Window_get_windows_above(Window *parent, Window *child)
{
    int i;
    Window *current_window;
    List *return_list;

    if(!(return_list = List_new()))
        return return_list;

    if(child->flags & WIN_FLOATING)
    {
        return return_list;
    }

    for(i = 0; i < parent->children->count; ++i)
    {
        current_window = (Window *)List_get_at(parent->children, i);
        
        if(current_window->flags & WIN_MINIMIZED)
        {
            continue;
        }

        if(current_window->index >= child->index && !(current_window->flags & WIN_FLOATING))
        {
            continue;
        }

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

List *Window_get_windows_below(Window *parent, Window *child)
{
    int i;
    Window *current_window;
    List *return_list;

    if(!(return_list = List_new()))
    {
        return return_list;
    }

    for(i = 0; i < parent->children->count; ++i)
    {
        if(child == (Window *)List_get_at(parent->children, i))
        {
            break;
        }
    }

    for(i = 0; i < parent->children->count; ++i)
    {
        current_window = List_get_at(parent->children, i);

        if(current_window->flags & WIN_MINIMIZED)
        {
            continue;
        }

        if(current_window->index <= child->index || current_window->flags & WIN_FLOATING)
        {
            continue;
        }

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
    uint32_t current_index;
    int inner_x1;
    int inner_y1;
    int inner_x2;
    int inner_y2;
    int window_hit = 0;

    Window *current_child;
    Window *child;

    List *processing_windows;

    for(i = 0; i < window->children->count; ++i)
    {
        child = (Window *)List_get_at(window->children, i);

        if(!(child->flags & WIN_FLOATING))
        {
            continue;
        }

        if(child->flags & WIN_MINIMIZED)
        {
            continue;
        }

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
                if(mouse_x >= child->x + child->width - 24 && 
                   mouse_x < child->x + child->width - 8 &&
                   mouse_y >= child->y + 8 &&
                   mouse_y < child->y + 24)
                {
                    Window_request_close(child);
                    
                    window_hit = 1;
                }
                else if(!(window->flags & WIN_NO_DRAG))
                {
                    window->dragging = 1;
                    window->drag_off_x = mouse_x - child->x;
                    window->drag_off_y = mouse_y - child->y;
                    window->drag_child = child;
                    
                    window_hit = 1;
                    break;
                }
            } 
            
            if(!(child->flags & WIN_NODECORATION ) &&
               !(child->flags & WIN_NO_RESIZE) &&
               mouse_x >= child->x + child->width - 16 &&
               mouse_x < child->x + child->width &&
               mouse_y >= child->y + child->height - 16 && 
               mouse_y < child->y + child->height)
            {
                window->resizing = 1;
                window->drag_off_x = mouse_x - child->width;
                window->drag_off_y = mouse_y - child->height;
                
                window->drag_child = child;

                window_hit = 1;
                break;
            }
        }

        Window_process_mouse(child, mouse_x - child->x, mouse_y - child->y, mouse_buttons);
        window_hit = 1;
        break;
    }

    if(!(processing_windows = List_new()))
    {
        return;
    }
    
    if(!window_hit)
    {
        
        // Construct a new list with all windows in drawing order
        for(current_index = 0; current_index < window->children->count; ++current_index)
        {
            for(i = 0; i < window->children->count; ++i)
            {
                current_child = (Window *)List_get_at(window->children, i);
                
                if(current_child->index == current_index)
                {
                    List_add(processing_windows, current_child);
                    continue;
                }    
            }
        }
        
        for(i = 0; i < processing_windows->count; ++i)
        {
            child = (Window *)List_get_at(processing_windows, i);
            
            if(child->flags & WIN_MINIMIZED)
            {
                continue;
            }
            
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
                    if(mouse_x >= child->x + child->width - 24 && 
                       mouse_x < child->x + child->width - 8 &&
                       mouse_y >= child->y + 8 &&
                       mouse_y < child->y + 24)
                    {
                        Window_request_close(child);
                    }
                    else if(!(window->flags & WIN_NO_DRAG))
                    {
                        window->dragging = 1;
                        window->drag_off_x = mouse_x - child->x;
                        window->drag_off_y = mouse_y - child->y;
                        window->drag_child = child;
                        
                        break;
                    }
                } 
                
                if(!(child->flags & WIN_NODECORATION ) &&
                   !(child->flags & WIN_NO_RESIZE) &&
                   mouse_x >= child->x + child->width - 16 &&
                   mouse_x < child->x + child->width &&
                   mouse_y >= child->y + child->height - 16 && 
                   mouse_y < child->y + child->height)
                {
                    window->resizing = 1;
                    window->drag_off_x = mouse_x - child->width;
                    window->drag_off_y = mouse_y - child->height;
                    
                    window->drag_child = child;
                }
            }
            
            Window_process_mouse(child, mouse_x - child->x, mouse_y - child->y, mouse_buttons);
            break;
        }
    }
    
    if(!mouse_buttons)
    {
        window->dragging = 0;
        window->resizing = 0;
        window->drag_child = (Window *)0;
    }

    if(window->drag_child && window->dragging)
    {
        Window_move(
            window->drag_child, 
            mouse_x - window->drag_off_x,
            mouse_y - window->drag_off_y);
    }

    if(window->drag_child && window->resizing)
    {
        Window_resize(
            window->drag_child,
            mouse_x - window->drag_off_x,
            mouse_y - window->drag_off_y);
    }

    if(window->mousedown_function && mouse_buttons && !window->last_button_state)
    {
        window->mousedown_function(window, mouse_x, mouse_y);
    }

    window->last_button_state = mouse_buttons;

    while(processing_windows->count)
    {
        List_remove_at(processing_windows, 0);
    }

    free(processing_windows);
}

void Window_process_keyboard(
    Window *window,
    int key,
    int mods,
    int action)
{
    if(window->active_child)
    {
        Window_process_keyboard(window->active_child, key, mods, action);
    }
    else if(window->key_function)
    {
        window->key_function(window, key, mods, action);
    }
}

void Window_process_tick(Window *window, int ticks)
{
    int i;
    Window *child;

    for(i = 0; i < window->children->count; ++i)
    {
        child = (Window *)List_get_at(window->children, i);

        Window_process_tick(child, ticks);
    }

    if(window->tick_function)
    {
        window->tick_function(window, ticks);
    }
}

void Window_mousedown_handler(Window *window, int x, int y)
{
    return;
}

void Window_key_handler(Window *window, int key, int mods, int action)
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

    uint32_t new_index = window->children->count;

    if(!(new_window = Window_new(x, y, width, height, flags, window->context, new_index)))
    {
        return new_window;
    }

    if(!List_add(window->children, (void *)new_window))
    {
        free(new_window);
        return (Window *)0;
    }

    new_window->parent = window;

    Window_raise(new_window, 1);

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

    if(window->flags & WIN_FLOATING)
    {
        return;
    }

    parent = window->parent;

    if(parent->active_child && parent->active_child == window)
    {
        return;
    }

    last_active = parent->active_child;

    uint32_t window_index = window->index;

    for(i = 0; i < parent->children->count; ++i)
    {
        Window *wind = (Window *)List_get_at(parent->children, i);

        if(wind->index < window_index)
        {
            ++wind->index;
        }
    }

    window->index = 0;

    parent->active_child = window;

    if(!do_draw)
    {
        return;
    }

    Window_paint(window, (List *)0, 1);

    if(last_active)
    {
        Window_update_title(last_active);
    }

    if(window->flags & WIN_NODECORATION)
    {
        return;
    }

    Desktop_invalidate_start_bar(parent);

    //Window_paint(parent, (List *)0, 1);
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

    if(window->flags & WIN_NO_DRAG || window->flags & WIN_MAXIMIZED)
    {
        return;
    }

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

void Window_resize(Window *window, int new_width, int new_height)
{
    int i;
    int old_width = window->width;
    int old_height = window->height;
    
    Rect new_window_rect;
    List *replacement_list;
    List *dirty_list;
    List *dirty_windows;

    if(window->flags & WIN_NO_RESIZE || window->flags & WIN_MAXIMIZED)
    {
        return;
    }

    // Make sure that constraints of the window is held
    if(new_width < window->min_width)
    {
        new_width = window->min_width;
    }

    if(new_height < window->min_height)
    {
        new_height = window->min_height;
    }

    if(new_width > window->max_width)
    {
        new_width = window->max_width;
    }

    if(new_height > window->max_height)
    {
        new_height = window->max_height;
    }
    
    Window_raise(window, 0);
    
    Window_apply_bound_clipping(window, 0, (List *)0);

    window->width = new_width;
    window->height = new_height;

    new_window_rect.top = Window_screen_y(window);
    new_window_rect.left = Window_screen_x(window);
    new_window_rect.bottom = new_window_rect.top + window->height - 1;
    new_window_rect.right = new_window_rect.left + window->width - 1;

    window->width = old_width;
    window->height = old_height;

    Context_subtract_clip_rect(window->context, &new_window_rect);

    if(!(replacement_list = List_new()))
    {
        Context_clear_clip_rects(window->context);
        return;
    }

    dirty_list = window->context->clip_rects;
    window->context->clip_rects = replacement_list;

    dirty_windows = Window_get_windows_below(window->parent, window);

    window->width = new_width;
    window->height = new_height;

    if(window->flags & WIN_NODECORATION)
    {
        window->inner_width = new_width;
        window->inner_height = new_height;
    }
    else
    {
        window->inner_width = new_width - 2*WIN_BORDERWIDTH;
        window->inner_height = new_height - WIN_TITLEHEIGHT - 2*WIN_BORDERWIDTH;
    }
    
    while(dirty_windows->count)
    {
        Window_paint((Window *)List_remove_at(dirty_windows, 0), dirty_list, 1);
    }

    Window_paint(window, (List *)0, 1);
    Window_paint(window->parent, dirty_list, 0);
    

    while(dirty_list->count)
    {
        free(List_remove_at(dirty_list, 0));
    }

    free(dirty_list);
    free(dirty_windows);
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

    Window_paint(window, dirty_regions, 1);


    List_remove_at(dirty_regions, 0);
    free(dirty_regions);
    free(dirty_rect);
}

void Window_set_title(Window *window, char *new_title)
{
    int len;

    // A realloc might be useful here...
    // libc has a realloc but libk might not.
    if(window->title)
    {
        free(window->title);
    }

    len = strlen(new_title);

    if(!(window->title = (char *)malloc((len + 1) * sizeof(char))))
    {
        return;
    }

    strncpy(window->title, new_title, len);
    
    window->title[len] = '\0';

    if(window->flags & WIN_NODECORATION)
    {
        Window_invalidate(window, 0, 0, window->height - 1, window->width - 1);
    }
    else
    {
        Window_update_title(window);
    }
}

void Window_append_title(Window *window, char *add_c)
{
    char *new_string;
    int orig_len;
    int add_len;

    if(!window->title)
    {
        Window_set_title(window, add_c);
    }

    orig_len = strlen(window->title);
    add_len = strlen(add_c);

    if(!(new_string = (char *)malloc(sizeof(char) * (orig_len + add_len + 1))))
    {
        return;
    }

    strncpy(new_string, window->title, orig_len);
    strncpy(new_string + orig_len, add_c, add_len);
    new_string[orig_len + add_len] = '\0';

    free(window->title);
    
    window->title = new_string;

    if(window->flags & WIN_NODECORATION)
    {
        Window_invalidate(window, 0, 0, window->height - 1, window->width - 1);
    }
    else
    {
        Window_update_title(window);
    }
}

void Window_get_title(Window *window, char *buf)
{
    if(!window->title)
    {
        *buf = '\0';
        return;
    }
    
    uint32_t len = strlen(window->title);

    strncpy(buf, window->title, len);

    buf[len] = '\0';
}

void Window_minimize(Window *window)
{
    window->flags |= (WIN_MINIMIZED);

    Window_invalidate(
        window->parent, 
        window->y, 
        window->x, 
        window->y + window->height - 1, 
        window->x + window->width - 1);

    // TODO: Find another window to raise by picking random or create
    // a list of newly used windows.

    window->parent->active_child = 0;
    Desktop_invalidate_start_bar(window->parent);
}

void Window_restore(Window *window)
{
    window->flags &= ~(WIN_MINIMIZED);

    Window_invalidate(
        window->parent, 
        window->y, 
        window->x, 
        window->y + window->height - 1, 
        window->x + window->width - 1);

    Window_raise(window, 1);
}

int Window_minimized(Window *window)
{
    return window->flags & WIN_MINIMIZED;
}

void Window_remove(Window *window)
{
    Window *parent;
    Window *child;
    int i;
    int window_index;

    parent = window->parent;

    for(i = 0; i < parent->children->count; ++i)
    {
        child = (Window *)List_get_at(parent->children, i);

        if(child == window)
        {
            window_index = i;

            break;
        }
    }

    while(window->children->count)
    {
        child = (Window *)List_remove_at(window->children, 0);

        Window_remove(child);
    }

    List_remove_at(parent->children, window_index);

    Window_invalidate(
        parent, 
        window->y, 
        window->x,
        window->y + 
        window->height - 1,
        window->x + window->width - 1);

    free(window->title);

    free(window);
}

void Window_close(Window *window)
{
    Desktop_remove_window(window->parent, window);
}

int Window_should_close(Window *window)
{
    return window->flags & WIN_SHOULD_CLOSE;
}

void Window_request_close(Window *window)
{
    window->flags |= WIN_SHOULD_CLOSE;
}

void Window_tick_handler(Window *window, int ticks)
{
    // Mark parameters as unused to avoid warnings.
    (void)window;
    (void)ticks;
    return;
}


int Window_is_focused(Window *window)
{
    if(!(window->parent))
    {
        return 0;
    }

    return window->parent->active_child == window;
}

float Window_get_aspect_ratio(Window *window)
{
    return (float)window->width / (float)window->height;
}

void Window_maximize(Window *window)
{
    int parent_width;
    int parent_height;
    
    // If we are not a child window, maximize does not make sense.
    if(!window->parent)
    {
        return;
    }

    // If the window is already maximized, we just have to return.
    if(window->flags & WIN_MAXIMIZED)
    {
        return;
    }

    // Store the current window size and position so we can restore it
    // later.
    window->last_x = window->x;
    window->last_y = window->y;
    window->last_width = window->width;
    window->last_height = window->height;

    parent_width = window->parent->inner_width;
    parent_height = window->parent->inner_height;

    // TODO: Use separate functions to avoid triggering repaint after each call.
    Window_move(window, 0, 0);
    Window_resize(window, parent_width, parent_height);

    window->flags |= WIN_MAXIMIZED;

    Desktop_invalidate_start_bar(window->parent);
    
    // TODO: Call onWindowMaximized here.
}

void Window_unmaximize(Window *window)
{
    // If we do not have a parent, return
    if(!window->parent)
    {
        return;
    }

    // If we are not maximized, we cannot do anything useful.
    if(!(window->flags & WIN_MAXIMIZED))
    {
        return;
    }
    
    window->flags &= ~(WIN_MAXIMIZED);

    Window_move(window, window->last_x, window->last_y);
    Window_resize(window, window->last_width, window->last_height);
}

/* window.c ends here */
