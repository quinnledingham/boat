#include <glad.h>
#include <glad.c>
#include <SDL.h>

#include "types.h"

#define function static
#define local_persist static
#define global_variable static

global_variable SDL_Window* window;

function void
do_one_frame()
{
    
}

function void
main_loop()
{
    v2s window_dim;
    SDL_GetWindowSize(window, &window_dim.Width, &window_dim.Height);
    
    while(1)
    {
        SDL_Event event;
        while(SDL_PollEvent(&event))
        {
            switch(event.type)
            {
                case SDL_WINDOWEVENT_RESIZED:
                {
                    glViewport(0, 0, event.window.data1, event.window.data2);
                } break;
                
                case SDL_QUIT:
                {
                    return;
                } break;
            }
        }
        
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
        
        do_one_frame();
        
        SDL_GL_SwapWindow(window);
    }
}

function void
init_opengl()
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
    window = SDL_CreateWindow("Boat", 
                              SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 
                              800, 800, 
                              SDL_WINDOW_RESIZABLE | SDL_WINDOW_OPENGL);
    
    init_opengl();
    main_loop();
    
    return 0;
}