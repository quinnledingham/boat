#include <stdio.h>

#include <glad.h>
#include <glad.c>
#include <SDL.h>

#include "types.h"

#define function static
#define local_persist static
#define global_variable static

#include "log.cpp"

#include "rend.h"
#include "rend.cpp"

struct Controller
{
    
};

struct Storage
{
    Mesh rect;
    Shader color_shader;
};

struct Platform
{
    v2s window_dim;
    
    Controller controllers[5];
    
    void *storage;
    
    b32 initialized;
};

function void
do_one_frame(Platform *platform)
{
    Storage *storage = (Storage*)platform->storage;
    if (!platform->initialized)
    {
        platform->storage = SDL_malloc(sizeof(Storage));
        storage = (Storage*)platform->storage;
        
        storage->color_shader.vs_filename = "../data/color.vs";
        storage->color_shader.fs_filename = "../data/color.fs";
        load_opengl_shader(&storage->color_shader);
        
        init_rect_mesh(&storage->rect);
        
        platform->initialized = true;
    }
    
    glViewport(0, 0, platform->window_dim.Width, platform->window_dim.Height);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
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
    Platform platform = {};
    SDL_GetWindowSize(window, &platform.window_dim.Width, &platform.window_dim.Height);
    
    while(1)
    {
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
                            platform.window_dim.Width = event.window.data1;
                            platform.window_dim.Height = event.window.data2;
                        } break;
                    }
                } break;
                
                case SDL_QUIT:
                {
                    return;
                } break;
            }
        }
        
        do_one_frame(&platform);
        
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
}

int main(int argc, char* argv[])
{
    SDL_Init(SDL_INIT_VIDEO | SDL_INIT_GAMECONTROLLER | SDL_INIT_HAPTIC | SDL_INIT_AUDIO);
    
    SDL_Window *window = SDL_CreateWindow("Boat", 
                                          SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 
                                          800, 800, 
                                          SDL_WINDOW_RESIZABLE | SDL_WINDOW_OPENGL);
    sdl_init_opengl(window);
    main_loop(window);
    
    return 0;
}