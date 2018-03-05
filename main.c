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

Window icon
Window Maximize 

Additional flags
 - Focused

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
 - Command line parser/terminal
 - File explorer
 - Control Panel
 - Calendar
 - Text editor

Secondary goals:

Multiple themes
Change background

Multiple fonts
 - Get advance and height and such from fonts
 - Read proper font files / FreeType library

Start Menu

Create a pipe or socket to communicate to external programs. 
 - Syscall-like syntax is preferred to emulate OS environment.

Native bitmap/PNG support

Basic Control Panel
 - Change Theme
 - Change Background
 - Change time format
 - Change default font

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
#include "textbox.h"
#include <pthread.h>

Desktop *desktop;
int quit = 0;

void main_mouse_callback(uint16_t mouse_x, uint16_t mouse_y, uint8_t buttons)
{
    pthread_mutex_lock(&desktop->mutex);
    Desktop_process_mouse(desktop, mouse_x, mouse_y, buttons);
    pthread_mutex_unlock(&desktop->mutex);
}

void main_keyboard_callback(int key, int mods, int action)
{
    pthread_mutex_lock(&desktop->mutex);
    Window_process_mouse((Window *)desktop, key, mods, action);
    pthread_mutex_unlock(&desktop->mutex);   
}

void tick_thread_func()
{
    int ticks = 0;

    while(!quit)
    {
        // TODO: More robust tick system for counting ticks.
        usleep(50000);

        ticks = 50;

        pthread_mutex_lock(&desktop->mutex);
        Window_process_tick((Window *)desktop, ticks);
        pthread_mutex_unlock(&desktop->mutex);
    }
}

int main(int argc, char **argv)
{
    SDL_Event event;

    uint32_t screen_width = 1024;
    uint32_t screen_height = 768;
   
    SDL_Window *sdl_window = NULL;
    SDL_Renderer *renderer = NULL;
    SDL_Texture *texture  = NULL;
    uint32_t *framebuffer = NULL;

    int buttonState = 0;

    int cascade_index_x = 0;
    int cascade_index_y = 0;

    Context *context;

    pthread_t tick_thread;

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

    context = Context_new(0,0,0);

    context->buffer = framebuffer;
    context->width = screen_width;
    context->height = screen_height;

    desktop = Desktop_new(context);
    
    Desktop_create_window(
        (Window *)desktop, 
        10, 
        10, 
        300, 
        200, 
        0, 
        "Window 1");

    Window *window = Desktop_create_window(
        (Window *)desktop, 
        100, 
        150, 
        400, 
        400, 
        0, 
        "Button Window");
    
    Desktop_create_window(
        (Window *)desktop, 
        200, 
        100, 
        200, 
        600, 
        0, 
        "Window 2");

    Button *button = Button_new(307, 357, 80, 30);
    Window_insert_child(window, (Window *)button);
    Window_set_title((Window *)button, "Button1");

    Button *button2 = Button_new(307, 257, 80, 30);
    Window_insert_child(window, (Window *)button2);
    Window_set_title((Window *)button2, "Button2");

    TextBox *textbox = TextBox_new(16, 48, 200, 30);
    Window_insert_child(window, (Window *)textbox);
    Window_set_title((Window *)textbox, "Enter text");

    Window_paint((Window *)desktop, (List *)0, 1);

    if(pthread_create(&tick_thread, NULL, tick_thread_func, NULL))
    {
        sprintf(stderr, "Failed to create tick thread");
    }

    int got_event = 0;

    while(!quit)
    {

        got_event = SDL_WaitEventTimeout(&event, 100);
            
        int mouse_x;
        int mouse_y;
        
        if(got_event)
        {
            
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
                        case SDLK_n:
                            Desktop_create_window(
                                (Window *)desktop, 
                                10 + 16*cascade_index_x, 
                                10 + 16*cascade_index_y, 
                                300, 
                                200, 
                                0, 
                                "Random Window");
                            
                            ++cascade_index_x;
                            ++cascade_index_y;
                            
                            cascade_index_x %= 10;
                            cascade_index_y %= 5;
                            
                            
                            break;
                        case SDLK_k:
                            {
                                Window *desktop_window = (Window *)desktop;
                                
                                if(desktop_window->active_child)
                                    Desktop_remove_window(
                                        desktop_window, 
                                        desktop_window->active_child);
                            }
                            break;
                    }
                    
                    main_keyboard_callback(
                        event.key.keysym.sym,
                        event.key.keysym.mod,
                        0);
                    
                    break;
                    
                case SDL_KEYUP:
                    
                    main_keyboard_callback(
                        event.key.keysym.sym,
                        event.key.keysym.mod,
                        0);
                    
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
        }
        
        SDL_UpdateTexture(texture, NULL, framebuffer, screen_width*sizeof(uint32_t));
        
        SDL_RenderClear(renderer);
        SDL_RenderCopy(renderer, texture, NULL, NULL);
        SDL_RenderPresent(renderer);
    }
    
    pthread_join(tick_thread, NULL);
    
    free(desktop);
    free(framebuffer);

    SDL_DestroyTexture(texture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(sdl_window);
    SDL_Quit();

    return 0;
}

/* "'(file-name-nondirectory (buffer-file-name))'" ends here */
