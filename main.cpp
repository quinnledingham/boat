#include <stdio.h>

#include <glad.h>
#include <glad.c>
#include <SDL.h>

#include "types.h"
#include "log.h"
#include "math.h"
#include "rend.h"

struct Camera
{
    v3 position;
    v3 target;
    v3 up;
    real32 fov;
    real32 yaw;
    real32 pitch;
};

#include "application.h"

#include "rend.cpp"

function void
update_camera_with_mouse(Camera *camera, v2s mouse)
{
    if (mouse.x > 1 || mouse.x < -1)
        camera->yaw += (f32)mouse.x * 0.1f;
    if (mouse.y > 1 || mouse.y < -1)
        camera->pitch -= (f32)mouse.y * 0.1f;
    
    if (camera->pitch > 89.0f)
        camera->pitch = 89.0f;
    if (camera->pitch < -89.0f)
        camera->pitch = -89.0f;
    
    v3 camera_direction = {
        cosf(DEG2RAD * camera->yaw) * cosf(DEG2RAD * camera->pitch),
        sinf(DEG2RAD * camera->pitch),
        sinf(DEG2RAD * camera->yaw) * cosf(DEG2RAD * camera->pitch)
    };
    camera->target = normalized(camera_direction);
}

function void
initialize_storage(Storage* storage)
{
    storage->camera.position = {0, 0, 2};
    storage->camera.up = {0, 1, 0};
    storage->camera.target = {0, 0, -2};
    storage->camera.yaw = -90.0f;
    
    storage->color_2D.vs_filename = "../data/color2D.vs";
    storage->color_2D.fs_filename = "../data/color.fs";
    load_opengl_shader(&storage->color_2D);
    
    storage->color_3D.vs_filename = "../data/color3D.vs";
    storage->color_3D.fs_filename = "../data/color.fs";
    load_opengl_shader(&storage->color_3D);
    
    init_rect_mesh(&storage->rect);
}

function void
do_one_frame(Application *app)
{
    Storage *storage = &app->storage;
    Controller *controller = &app->controller;
    
    if (on_down(controller->pause))
        app->paused = !app->paused;
    
    if (!app->paused)
    {
        SDL_SetRelativeMouseMode(SDL_TRUE);
        update_camera_with_mouse(&storage->camera, controller->mouse);
    }
    else if (app->paused)
    {
        SDL_SetRelativeMouseMode(SDL_FALSE);
    }
    
    r32 aspect_ratio = (r32)app->window_dim.Width / (r32)app->window_dim.Height;
    m4x4 perspective_matrix = perspective_projection(90.0f, aspect_ratio, 0.01f, 1000.0f);
    m4x4 orthographic_matrix = orthographic_projection(0.0f, (r32)app->window_dim.Width, (r32)app->window_dim.Height,
                                                       0.0f, -3.0f, 3.0f);
    m4x4 view_matrix = look_at(storage->camera.position, 
                               storage->camera.position + storage->camera.target,
                               storage->camera.up);
    
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
    
    if (on_down(controller->up))
    {
        print_m4x4(perspective_matrix);
    }
    
    u32 active_shader = use_shader(&storage->color_3D);
    v4 color = {255, 0, 0, 1};
    m4x4 model = create_transform_m4x4({0, 0, 0}, {}, {1, 1, 1});
    glUniform4fv(glGetUniformLocation(active_shader, "user_color"), (GLsizei)1, (float*)&color);
    glUniformMatrix4fv(glGetUniformLocation(active_shader, "model"), (GLsizei)1, false, (float*)&model);
    glUniformMatrix4fv(glGetUniformLocation(active_shader, "projection"), (GLsizei)1, false, (float*)&perspective_matrix);
    glUniformMatrix4fv(glGetUniformLocation(active_shader, "view"), (GLsizei)1, false, (float*)&view_matrix);
    opengl_draw_mesh(&storage->rect);
    glUseProgram(0);
    
    if (app->paused)
    {
        u32 active_shader = use_shader(&storage->color_2D);
        v4 color = {0, 0, 0, 0.7};
        m4x4 model = create_transform_m4x4({0, 0, 0}, 
                                           {}, {(f32)app->window_dim.Width, (f32)app->window_dim.Height, 1});
        glUniform4fv(glGetUniformLocation(active_shader, "user_color"), (GLsizei)1, (float*)&color);
        glUniformMatrix4fv(glGetUniformLocation(active_shader, "model"), (GLsizei)1, false, (float*)&model);
        glUniformMatrix4fv(glGetUniformLocation(active_shader, "projection"), (GLsizei)1, false, (float*)&orthographic_matrix);
        opengl_draw_mesh(&storage->rect);
        glUseProgram(0);
    }
}

function void
main_loop(SDL_Window *window)
{
    Application app = {};
    SDL_GetWindowSize(window, &app.window_dim.Width, &app.window_dim.Height);
    SDL_SetRelativeMouseMode(SDL_TRUE);
    
    Controller *controller = &app.controller;
    controller->right.id = SDLK_RIGHT;
    controller->up.id = SDLK_UP;
    controller->left.id = SDLK_LEFT;
    controller->down.id = SDLK_DOWN;
    controller->pause.id = SDLK_ESCAPE;
    
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
                
                case SDL_MOUSEMOTION:
                {
                    controller->mouse.x = event.motion.xrel;
                    controller->mouse.y = event.motion.yrel;
                } break;
                
                case SDL_KEYDOWN:
                case SDL_KEYUP:
                {
                    s32 key_id = event.key.keysym.sym;
                    b32 state = false;
                    if (event.key.state == SDL_PRESSED)
                        state = true;
                    
                    for (int i = 0; i < array_count(controller->buttons); i++)
                    {
                        if (key_id == controller->buttons[i].id)
                        {
                            controller->buttons[i].current_state = state;
                            break;
                        }
                    }
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
    glEnable(GL_BLEND);
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