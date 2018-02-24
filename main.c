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

#include <SDL2/SDL.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <time.h>

#include "context.h"
#include "desktop.h"

Desktop *desktop;

void main_mouse_callback(uint16_t mouse_x, uint16_t mouse_y, uint8_t buttons)
{
    Desktop_process_mouse(desktop, mouse_x, mouse_y, buttons);
}

int main(int argc, char **argv)
{
    int quit = 0;
    SDL_Event event;

    uint32_t screen_width = 1024;
    uint32_t screen_height = 768;
   
    SDL_Window *window = NULL;
    SDL_Renderer *renderer = NULL;
    SDL_Texture *texture  = NULL;
    uint32_t *framebuffer = NULL;

    int buttonState = 0;

    SDL_Init(SDL_INIT_VIDEO);

    window = SDL_CreateWindow(
        "SDL2 Pixel Drawing", 
        SDL_WINDOWPOS_UNDEFINED, 
        SDL_WINDOWPOS_UNDEFINED, 
        screen_width, 
        screen_height, 
        0);

    renderer = SDL_CreateRenderer(
        window,
        -1, 
        SDL_RENDERER_ACCELERATED);

    texture = SDL_CreateTexture(
        renderer, 
        SDL_PIXELFORMAT_ARGB8888, 
        SDL_TEXTUREACCESS_STATIC, 
        screen_width, 
        screen_height);
   
    // TODO: Better allocator than malloc
    framebuffer = malloc(sizeof(uint32_t) * screen_width * screen_height);

    memset(framebuffer, 0, screen_width * screen_height * sizeof(uint32_t));

    Context *context = Context_new(0,0,0);

    context->buffer = framebuffer;
    context->width = screen_width;
    context->height = screen_height;

    desktop = Desktop_new(context);
    
    Desktop_create_window(desktop, 10, 10, 300, 200);
    Desktop_create_window(desktop, 100, 150, 400, 400);
    Desktop_create_window(desktop, 200, 100, 200, 600);

    Desktop_paint(desktop);
   
    while(!quit)
    {
        SDL_UpdateTexture(texture, NULL, framebuffer, screen_width*sizeof(uint32_t));

        SDL_WaitEvent(&event);

        int mouse_x;
        int mouse_y;
        
        switch(event.type)
        {
            case SDL_QUIT:
                quit = 1;
                break;
            case SDL_MOUSEBUTTONUP:
                if(event.button.button == SDL_BUTTON_LEFT)    
                    buttonState &= ~(1<<0);
                if(event.button.button == SDL_BUTTON_RIGHT)
                    buttonState &= ~(1<<1);

                SDL_GetMouseState(&mouse_x, &mouse_y);

                main_mouse_callback((uint16_t) mouse_x, (uint16_t) mouse_y, buttonState);
                
                break;
            case SDL_MOUSEBUTTONDOWN:
                if(event.button.button == SDL_BUTTON_LEFT)    
                    buttonState |= (1<<0);
                if(event.button.button == SDL_BUTTON_RIGHT)
                    buttonState |= (1<<1);
                
                SDL_GetMouseState(&mouse_x, &mouse_y);

                main_mouse_callback((uint16_t) mouse_x, (uint16_t) mouse_y, buttonState);
                
                break;

            case SDL_MOUSEMOTION:                
                SDL_GetMouseState(&mouse_x, &mouse_y);

                main_mouse_callback((uint16_t) event.motion.x, (uint16_t) event.motion.y, buttonState);
                
                break;
        }

        SDL_RenderClear(renderer);
        SDL_RenderCopy(renderer, texture, NULL, NULL);
        SDL_RenderPresent(renderer);
    }

    free(desktop);
    free(framebuffer);

    SDL_DestroyTexture(texture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}

/* "'(file-name-nondirectory (buffer-file-name))'" ends here */
