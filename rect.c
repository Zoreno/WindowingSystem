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

#include "rect.h"
#include <stdlib.h>

Rect *Rect_new(int top, int left, int bottom, int right)
{
    Rect *rect;
    if(!(rect = (Rect *)malloc(sizeof(Rect))))
    {
        return rect;
    }

    rect->top = top;
    rect->left = left;
    rect->bottom = bottom;
    rect->right = right;

    return rect;
}

List *Rect_split(Rect *subject_rect, Rect *cutting_rect)
{
    List *output_rects;

    if(!(output_rects = List_new()))
        return output_rects;

    Rect subject_copy;
    subject_copy.top = subject_rect->top;
    subject_copy.left = subject_rect->left;
    subject_copy.bottom = subject_rect->bottom;
    subject_copy.right = subject_rect->right;

    Rect *temp_rect;

    if(cutting_rect->left > subject_copy.left &&
       cutting_rect->left <= subject_copy.right)
    {
        if(!(temp_rect = Rect_new(
                 subject_copy.top, 
                 subject_copy.left, 
                 subject_copy.bottom, 
                 cutting_rect->left - 1)))
        {
            free(output_rects);
            
            return (List *)0;
        }

        List_add(output_rects, temp_rect);

        subject_copy.left = cutting_rect->left;
    }

    if(cutting_rect->top > subject_copy.top &&
       cutting_rect->top <= subject_copy.bottom)
    {
        if(!(temp_rect = Rect_new(
                 subject_copy.top,
                 subject_copy.left,
                 cutting_rect->top - 1,
                 subject_copy.right)))
        {
            while(output_rects->count)
            {
                temp_rect = List_remove_at(output_rects, 0);

                free(temp_rect);
            }

            free(output_rects);

            return (List *)0;
        }

        List_add(output_rects, temp_rect);

        subject_copy.top = cutting_rect->top;
    }

    if(cutting_rect->right >= subject_copy.left && 
       cutting_rect->right < subject_copy.right)
    {
        if(!(temp_rect = Rect_new(
                 subject_copy.top,
                 cutting_rect->right + 1,
                 subject_copy.bottom,
                 subject_copy.right)))
        {
            while(output_rects->count)            
            {
                temp_rect = List_remove_at(output_rects, 0);
                free(temp_rect);
            }

            free(output_rects);

            return (List *)0;
        }

        List_add(output_rects, temp_rect);

        subject_copy.right = cutting_rect->right;
    }

    if(cutting_rect->bottom >= subject_copy.top &&
       cutting_rect->bottom < subject_copy.bottom)
    {
        if(!(temp_rect = Rect_new(
                 cutting_rect->bottom + 1,
                 subject_copy.left,
                 subject_copy.bottom,
                 subject_copy.right)))
        {
            while(output_rects->count)
            {
                temp_rect = List_remove_at(output_rects, 0);
                free(temp_rect);
            }

            free(output_rects);

            return (List *)0;
        }

        List_add(output_rects, temp_rect);

        subject_copy.bottom = cutting_rect->bottom;
    }

    return output_rects;
}

Rect *Rect_intersect(Rect *rect_a, Rect *rect_b)
{
    Rect *result_rect;

    if(!(rect_a->left <= rect_b->right && 
       rect_a->right >= rect_b->left &&
       rect_a->top <= rect_b->bottom && 
       rect_a->bottom >= rect_b->top))
    {
        return (Rect *)0;
    }

    if(!(result_rect = Rect_new(rect_a->top, rect_a->left, rect_a->bottom, rect_a->right)))
    {
        return (Rect *)0;
    }

    if(rect_b->left >= result_rect->left && rect_b->left <=result_rect->right)
    {
        result_rect->left = rect_b->left;
    }

    if(rect_b->top >= result_rect->top && rect_b->top <= result_rect->bottom)
    {
        result_rect->top = rect_b->top;
    }

    if(rect_b->right >= result_rect->left && rect_b->right <= result_rect->right)
    {
        result_rect->right = rect_b->right;
    }

    if(rect_b->bottom >= result_rect->top && rect_b->bottom <= result_rect->bottom)
    {
        result_rect->bottom = rect_b->bottom;
    }

    return result_rect;
}

/* "'(file-name-nondirectory (buffer-file-name))'" ends here */
