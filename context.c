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
#include "context.h"

Context *Context_new(uint16_t width, uint16_t height, uint32_t *buffer)
{
    Context *context;

    if(!(context = (Context *)malloc(sizeof(Context))))
    {
        return context;
    }

    if(!(context->clip_rects = List_new()))
    {
        free(context);
        return (Context *)0;
    }

    context->width = width;
    context->height = height;
    context->buffer = buffer;

    return context;
}

void Context_clipped_rect(
    Context *context,
    int x,
    int y,
    unsigned int width,
    unsigned int height,
    Rect *clip_area, 
    uint32_t color)
{
    int cur_x;
    int max_x = x + width;
    int max_y = y + height;

    x += context->translate_x;
    y += context->translate_y;

    max_x += context->translate_x;
    max_y += context->translate_y;

    if(x < clip_area->left)
        x = clip_area->left;

    if(y < clip_area->top)
        y = clip_area->top;

    if(max_x > clip_area->right + 1)
        max_x = clip_area->right + 1;

    if(max_y > clip_area->bottom + 1)
        max_y = clip_area->bottom + 1;

    for(; y < max_y; ++y)
    {
        for(cur_x = x; cur_x < max_x; ++cur_x)
        {
            context->buffer[y * context->width + cur_x] = color;
        }
    }
}

void Context_fill_rect(
    Context *context, 
    int x, 
    int y,
    unsigned int width,
    unsigned int height, 
    uint32_t color)
{
    int start_x;
    int cur_x;
    int cur_y;
    int end_x;
    int end_y;
    
    int max_x = x + width;
    int max_y = y + height;

    int i;

    Rect *clip_area;
    Rect screen_area;

    if(context->clip_rects->count)
    {
        for(i = 0; i < context->clip_rects->count; ++i)
        {
            clip_area = (Rect *)List_get_at(context->clip_rects, i);
            Context_clipped_rect(context, x, y, width, height, clip_area, color);
        }
    }
    else
    {
        screen_area.top = 0;
        screen_area.left = 0;
        screen_area.bottom = context->height - 1;
        screen_area.right = context->width - 1;
        Context_clipped_rect(context, x, y, width, height, &screen_area, color);
    }
}

void Context_horizontal_line(
    Context *context, 
    int x, 
    int y, 
    unsigned int length, 
    uint32_t color)
{
    Context_fill_rect(context, x, y, length, 1, color);
}

void Context_vertical_line(
    Context *context, 
    int x, 
    int y, 
    unsigned int length, 
    uint32_t color)
{
    Context_fill_rect(context, x, y, 1, length, color);
}

void Context_draw_rect(
    Context *context, 
    int x, 
    int y, 
    unsigned int width, 
    unsigned int height, 
    uint32_t color)
{
    Context_horizontal_line(context, x, y, width, color);
    Context_vertical_line(context, x, y + 1, height - 2, color);
    Context_horizontal_line(context, x, y + height - 1, width, color);
    Context_vertical_line(context, x + width - 1, y + 1, height - 2, color);
}

void Context_intersect_clip_rect(Context *context, Rect *rect)
{
    int i;
    List *output_rects;
    Rect *current_rect;
    Rect *intersect_rect;

    if(!(output_rects = List_new()))
    {
        return;
    }

    for(i = 0; i < context->clip_rects->count; ++i)
    {
        current_rect = (Rect *)List_get_at(context->clip_rects, i);
        intersect_rect = Rect_intersect(current_rect, rect);

        if(intersect_rect)
        {
            List_add(output_rects, intersect_rect);
        }
    }

    while(context->clip_rects->count)
    {
        List_remove_at(context->clip_rects, 0);
    }

    free(context->clip_rects);

    context->clip_rects = output_rects;

    free(rect);
}

void Context_subtract_clip_rect(
    Context *context, 
    Rect *subtracted_rect)
{
    int i, j;

    Rect *cur_rect;
    List *split_rects;

    for(i = 0; i < context->clip_rects->count;)
    {
        cur_rect = List_get_at(context->clip_rects, i);

        if(!(cur_rect->left <= subtracted_rect->right && 
             cur_rect->right >= subtracted_rect->left && 
             cur_rect->top <= subtracted_rect->bottom &&
             cur_rect->bottom >= subtracted_rect->top))
        {
            ++i;
            continue;
        }

        List_remove_at(context->clip_rects, i);
        split_rects = Rect_split(cur_rect, subtracted_rect);
        free(cur_rect);

        while(split_rects->count)
        {
            cur_rect = (Rect *)List_remove_at(split_rects, 0);
            List_add(context->clip_rects, cur_rect);
        }

        free(split_rects);

        i = 0;
    }
}

void Context_add_clip_rect(Context *context, Rect *added_rect)
{
    Context_subtract_clip_rect(context, added_rect);

    List_add(context->clip_rects, added_rect);
}

void Context_clear_clip_rects(Context *context)
{
    Rect *cur_rect;

    while(context->clip_rects->count)
    {
        cur_rect = (Rect *)List_remove_at(context->clip_rects, 0);
        free(cur_rect);
    }
}

/* "'(file-name-nondirectory (buffer-file-name))'" ends here */
