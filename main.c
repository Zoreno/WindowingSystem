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

/*
  
TODO List:

Primary goals:

Remove windows
Resize windows
Minimize windows
Window icon

Window Handle

Additional flags
 - Resizable
 - Visible
 - Decorated
 - Focused
 - Should close

Quality of life size/dimension accessors
 - Vectors
 - Bounding rect
 - Aspect ratio

Render bitmaps

Window Events
 - Key
 - Mouse Up
 - Mouse Down
 - Mouse Moved
 - Mouse Scroll
 - Gained focus
 - Lost focus
 - Iconified
 - Restored
 - Cursor enter
 - Resized

Simple controls
 - Button
 - Checkbox
 - Label
 - Textbox

Simple Applications
 - Calculator
 - Command line parser

Secondary goals:

Multiple themes

Multiple fonts
 - Get advance and height and such from fonts

Start bar

Create a pipe or socket to communicate to external programs. 
 - Syscall-like syntax is preferred to emulate OS environment.


 */

#include <SDL2/SDL.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <time.h>

#include "context.h"
#include "desktop.h"
#include "window.h"
#include "button.h"

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
   
    SDL_Window *sdl_window = NULL;
    SDL_Renderer *renderer = NULL;
    SDL_Texture *texture  = NULL;
    uint32_t *framebuffer = NULL;

    int buttonState = 0;

    SDL_Init(SDL_INIT_VIDEO);

    sdl_window = SDL_CreateWindow(
        "SDL2 Pixel Drawing", 
        SDL_WINDOWPOS_UNDEFINED, 
        SDL_WINDOWPOS_UNDEFINED, 
        screen_width, 
        screen_height, 
        0);

    renderer = SDL_CreateRenderer(
        sdl_window,
        -1, 
        SDL_RENDERER_ACCELERATED);

    texture = SDL_CreateTexture(
        renderer, 
        SDL_PIXELFORMAT_ARGB8888, 
        SDL_TEXTUREACCESS_STATIC, 
        screen_width, 
        screen_height);

    SDL_ShowCursor(SDL_DISABLE);
   
    // TODO: Better allocator than malloc
    framebuffer = malloc(sizeof(uint32_t) * screen_width * screen_height);

    memset(framebuffer, 0, screen_width * screen_height * sizeof(uint32_t));

    Context *context = Context_new(0,0,0);

    context->buffer = framebuffer;
    context->width = screen_width;
    context->height = screen_height;

    desktop = Desktop_new(context);
    
    Window_create_window((Window *)desktop, 10, 10, 300, 200, 0);
    Window *window = Window_create_window((Window *)desktop, 100, 150, 400, 400, WIN_MINIMIZED);
    Window_create_window((Window *)desktop, 200, 100, 200, 600, 0);

    Button *button = Button_new(307, 357, 80, 30);
    Window_insert_child(window, (Window *)button);
    Window_set_title((Window *)button, "Button1");

    Button *button2 = Button_new(307, 257, 80, 30);
    Window_insert_child(window, (Window *)button2);
    Window_set_title((Window *)button2, "Button2");

    Window_paint((Window *)desktop, (List *)0, 1);

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
            case SDL_KEYDOWN:
                switch(event.key.keysym.sym)
                {
                    case SDLK_e:
                        Window_minimize(window);
                        break;
                    case SDLK_r:
                        Window_restore(window);
                        break;
                }
                
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
    SDL_DestroyWindow(sdl_window);
    SDL_Quit();

    return 0;
}

/* "'(file-name-nondirectory (buffer-file-name))'" ends here */
