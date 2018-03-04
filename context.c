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
#include <string.h>
#include "context.h"
#include "font.h"

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
    context->clipping_on = 0;

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
        if(!context->clipping_on)
        {
            screen_area.top = 0;
            screen_area.left = 0;
            screen_area.bottom = context->height - 1;
            screen_area.right = context->width - 1;
            Context_clipped_rect(context, x, y, width, height, &screen_area, color);
        }
    }
}

void Context_clipped_bitmap(
    Context *context,
    int source_x,
    int source_y,
    int dest_x,
    int dest_y,
    unsigned int width,
    unsigned int height,
    Rect *clip_area,
    unsigned int *source)
{
    int cur_dest_x;
    int cur_source_x;
    int max_x = dest_x + width;
    int max_y = dest_y + height;

    dest_x += context->translate_x;
    dest_y += context->translate_y;

    max_x += context->translate_x;
    max_y += context->translate_y;

    if(dest_x < clip_area->left)
    {
        source_x += clip_area->left - dest_x;
        dest_x = clip_area->left;
    }

    if(dest_y < clip_area->top)
    {
        source_y += clip_area->top - dest_y;
        dest_y = clip_area->top;
    }

    if(max_x > clip_area->right + 1)
    {
        max_x = clip_area->right + 1;
    }

    if(max_y > clip_area->bottom + 1)
    {
        max_y = clip_area->bottom + 1;
    }

    // TODO: Make sure that source_x and source_y is within image and such


    for(; dest_y < max_y; ++dest_y, ++source_y)
    {
        for(cur_dest_x = dest_x, cur_source_x = source_x; 
            cur_source_x < max_x; 
            ++cur_dest_x, ++cur_source_x)
        {
            context->buffer[dest_y * context->width + cur_dest_x] = 
                source[source_y * width + cur_source_x];
        }
    }
}

// TODO: Implement scaling with different source and dest width/height
void Context_fill_bitmap(
    Context *context,
    int source_x, 
    int source_y,
    int dest_x,
    int dest_y,
    unsigned int width, 
    unsigned int height, 
    unsigned int *source)
{
    int i;

    Rect *clip_area;
    Rect screen_area;

    if(context->clip_rects->count)
    {
        for(i = 0; i < context->clip_rects->count; ++i)
        {
            clip_area = (Rect *)List_get_at(context->clip_rects, i);
            Context_clipped_bitmap(context, source_x, source_y, dest_x, 
                                   dest_y, width, height, clip_area, source);
        }
    }
    else
    {
        if(!context->clipping_on)
        {
            screen_area.top = 0;
            screen_area.left = 0;
            screen_area.bottom = context->height - 1;
            screen_area.right = context->width - 1;
            Context_clipped_bitmap(context, source_x, source_y, dest_x,
                                   dest_y, width, height, &screen_area, source);
        }
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

    context->clipping_on = 1;

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
        free(List_remove_at(context->clip_rects, 0));
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

    context->clipping_on = 1;

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

    context->clipping_on = 0;

    while(context->clip_rects->count)
    {
        cur_rect = (Rect *)List_remove_at(context->clip_rects, 0);
        free(cur_rect);
    }
}

void Context_draw_char_clipped(Context *context, char c, int x, int y, uint32_t color, Rect *bound_rect)
{
    int font_x;
    int font_y;
    int off_x = 0;
    int off_y = 0;
    int count_x = 8;
    int count_y = 12;
    uint8_t shift_line;

    x += context->translate_x;
    y += context->translate_y;

    c &= 0x7F;

    if(x > bound_rect->right || 
       (x + 8) <= bound_rect->left || 
       y > bound_rect->bottom || 
       (y + 12) <= bound_rect->top)
    {
        return;
    }

    if(x < bound_rect->left)
    {
        off_x = bound_rect->left - x;
    }

    if((x + 8) > bound_rect->right)
    {
        count_x = bound_rect->right - x + 1;
    }

    if(y < bound_rect->top)
    {
        off_y = bound_rect->top - y;;
    }

    if((y + 12) > bound_rect->bottom)
    {
        count_y = bound_rect->bottom - y + 1;
    }

    for(font_y = off_y; font_y < count_y; ++font_y)
    {
        shift_line = font_array[font_y * 128 + c];

        shift_line <<= off_x;

        for(font_x = off_x; font_x < count_x; ++font_x)
        {
            if(shift_line & 0x80)
            {
                context->buffer[(font_y + y) * context->width + (font_x + x)] = color;
            }

            shift_line <<= 1;
        }
    }
}

void Context_draw_char(Context *context, char c, int x, int y, uint32_t color)
{
    int i;
    Rect *clip_area;
    Rect screen_area;

    if(context->clip_rects->count)
    {
        for(i = 0; i < context->clip_rects->count; ++i)
        {
            clip_area = (Rect *)List_get_at(context->clip_rects, i);
            Context_draw_char_clipped(context, c, x, y, color, clip_area);
        }
    }
    else
    {
        if(!context->clipping_on)
        {
            screen_area.top = 0;
            screen_area.left = 0;
            screen_area.bottom = context->height - 1;
            screen_area.right = context->width - 1;
            Context_draw_char_clipped(context, c, x, y, color, clip_area);
        }
    }
}

void Context_draw_text(Context *context, char *string, int x, int y, uint32_t color)
{

    if(!string)
    {
        return;
    }

    for(; *string; x += 8)
    {
        Context_draw_char(context, *(string++), x, y, color);
    }
}

/* "'(file-name-nondirectory (buffer-file-name))'" ends here */
