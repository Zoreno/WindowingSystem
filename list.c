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

#include "list.h"


List *List_new()
{
    List *list;
    
    if(!(list = (List *)malloc(sizeof(List))))
        return list;

    list->count = 0;
    list->root_node = (ListNode *)0;

    return list;
}

int List_add(List *list, void *payload)
{
    ListNode *new_node;
    if(!(new_node = ListNode_new(payload)))
        return 0;
    
    if(!list->root_node)
    {
        list->root_node = new_node;
    }
    else
    {
        ListNode *current_node = list->root_node;

        while(current_node->next)
        {
            current_node = current_node->next;
        }

        current_node->next = new_node;
        new_node->prev = current_node;
    }

    list->count++;

    return 1;
}

void *List_get_at(List *list, unsigned int index)
{
    if(list->count == 0 || index >= list->count)
    {
        return (void *)0;
    }

    ListNode *current_node = list->root_node;
    
    for(unsigned int current_index = 0; (current_index < index && current_node); ++current_index)
    {
        current_node = current_node->next;
    }

    return current_node ? current_node->payload : (void *)0;
}

void *List_remove_at(List *list, unsigned int index)
{
    void *payload;
    
    if(list->count == 0 || index >= list->count)
        return (void *)0;

    ListNode *current_node = list->root_node;
    
    for(unsigned int current_index = 0; (current_index < index) && current_node; current_index++)
        current_node = current_node->next;

    if(!current_node)
        return (void *)0;

    payload = current_node->payload;

    if(current_node->prev)
        current_node->prev->next = current_node->next;

    if(current_node->next)
        current_node->next->prev = current_node->prev;

    if(index == 0)
        list->root_node = current_node->next;

    free(current_node);

    list->count--;

    return payload;
}


/* "'(file-name-nondirectory (buffer-file-name))'" ends here */
