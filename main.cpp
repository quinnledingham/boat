#include <stdio.h>

#include <glad.h>
#include <glad.c>
#include <SDL.h>

#include "types.h"
#include "rend.h"
#include "application.h"

#include "log.cpp"
#include "rend.cpp"

function void
initialize_storage(Storage* storage)
{
    storage->color_shader.vs_filename = "../data/color.vs";
    storage->color_shader.fs_filename = "../data/color.fs";
    load_opengl_shader(&storage->color_shader);
    init_rect_mesh(&storage->rect);
}

function void
do_one_frame(Application *app)
{
    Storage *storage = &app->storage;
    Controller *controller = &app->controller;
    
    if (is_down(controller->up))
    {
        
    }
    
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
    
    glUseProgram(storage->color_shader.handle);
    v4 color = {255, 0, 0, 1};
    glUniform4fv(glGetUniformLocation(storage->color_shader.handle, "user_color"), (GLsizei)1, (float*)&color);
    opengl_draw_mesh(&storage->rect);
    glUseProgram(0);
}

function void
main_loop(SDL_Window *window)
{
    Application app = {};
    SDL_GetWindowSize(window, &app.window_dim.Width, &app.window_dim.Height);
    
    Controller *controller = &app.controller;
    controller->right.id = SDLK_RIGHT;
    controller->up.id = SDLK_UP;
    controller->left.id = SDLK_LEFT;
    controller->down.id = SDLK_DOWN;
    
    while(1)
    {
        for (int i = 0; i < array_count(controller->buttons); i++)
            controller->buttons[i].previous_state =controller->buttons[i].current_state;
        
        SDL_Event event;
        while(SDL_PollEvent(&event))
        {
            switch(event.type)
            {
                case SDL_WINDOWEVENT:
                {
                    switch(event.window.event)
                    {
                        case SDL_WINDOWEVENT_RESIZED:
                        case SDL_WINDOWEVENT_SIZE_CHANGED:
                        {
                            app.window_dim.Width = event.window.data1;
                            app.window_dim.Height = event.window.data2;
                            glViewport(0, 0, app.window_dim.Width, app.window_dim.Height);
                        } break;
                    }
                } break;
                
                case SDL_KEYDOWN:
                case SDL_KEYUP:
                {
                    s32 key_id = event.key.keysym.sym;
                    b32 state = false;
                    if (event.key.state == SDL_PRESSED)
                        state = true;
                    
                    if (key_id == controller->right.id)
                        controller->right.current_state = state;
                    else if (key_id == controller->up.id)
                        controller->up.current_state = state;
                    else if (key_id == controller->left.id)
                        controller->left.current_state = state;
                    else if (key_id == controller->down.id)
                        controller->down.current_state = state;
                    
                } break;
                
                case SDL_QUIT:
                {
                    return;
                } break;
            }
        }
        
        if (!app.initialized)
        {
            Storage *storage = &app.storage;
            initialize_storage(storage);
            app.initialized = true;
        }
        do_one_frame(&app);
        
        SDL_GL_SwapWindow(window);
    }
}

function void
sdl_init_opengl(SDL_Window *window)
{
    SDL_GL_LoadLibrary(NULL);
    
    // Request an OpenGL 4.6 context (should be core)
    SDL_GL_SetAttribute(SDL_GL_ACCELERATED_VISUAL, 1);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 6);
    
    // Also request a depth buffer
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
    
    SDL_GLContext Context = SDL_GL_CreateContext(window);
    SDL_GL_SetSwapInterval(0);
    
    // Check OpenGL properties
    gladLoadGLLoader((GLADloadproc) SDL_GL_GetProcAddress);
    SDL_Log("OpenGL loaded\n");
    SDL_Log("Vendor:   %s", glGetString(GL_VENDOR));
    SDL_Log("Renderer: %s", glGetString(GL_RENDERER));
    SDL_Log("Version:  %s", glGetString(GL_VERSION));
    
    // Default settings
    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    v2s window_dim = {};
    SDL_GetWindowSize(window, &window_dim.Width, &window_dim.Height);
    glViewport(0, 0, window_dim.Width, window_dim.Height);
}

int main(int argc, char* argv[])
{
    SDL_Init(SDL_INIT_VIDEO | 
             SDL_INIT_GAMECONTROLLER | 
             SDL_INIT_HAPTIC | 
             SDL_INIT_AUDIO);
    
    SDL_Window *window = SDL_CreateWindow("Boat", 
                                          SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 
                                          800, 800, 
                                          SDL_WINDOW_RESIZABLE | SDL_WINDOW_OPENGL);
    sdl_init_opengl(window);
    main_loop(window);
    
    return 0;
}