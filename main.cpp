#include <stdio.h>

#include <glad.h>
#include <glad.c>
#include <SDL.h>

#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_RESIZE_IMPLEMENTATION
#define STB_TRUETYPE_IMPLEMENTATION
#include <stb_image.h>
#include <stb_image_resize.h>
#include <stb_truetype.h>

#include "types.h"
#include "log.h"
#include "math.h"
#include "string.h"
#include "rend.h"
#include "input.h"
#include "assets.h"
#include "application.h"
#include "sdl_keycodes.h"

#include "rend.cpp"

function void
free_mesh(Mesh *mesh)
{
    SDL_free(mesh->vertices);
    SDL_free(mesh->indices);
}

function Mesh
create_square_mesh(uint32 u, uint32 v)
{
    Mesh result = {};
    result.vertices_count = (u + 1) * (v + 1);
    result.vertices = (Vertex*)SDL_malloc(sizeof(Vertex) * result.vertices_count);
    
    f32 du = 2.0f / (f32)u;
    f32 dv = 2.0f / (f32)v;
    
    u32 vertex_count = 0;
    u32 s = 0, t = 0;
    for (u32 i = 0; i < (u + 1); i++, s += 2)
    {
        for (u32 j = 0; j < (v + 1); j++, t += 2)
        {
            v3 vertex_pos = { (f32(i) * du) - 1.0f, (f32)-1, (f32(j) * dv) - 1.0f };
            v2 tex_coords = { (f32)i, (f32)j };
            Vertex vertex = { vertex_pos, {0, 1, 0}, tex_coords };
            result.vertices[vertex_count++] = vertex;
        }
    }
    
    result.indices_count = u * v * 6;
    result.indices = (u32*)SDL_malloc(sizeof(u32) * result.indices_count);
    
    u32 indices_count = 0;
    for (u32 i = 0; i < u; i++)
    {
        u32 p1 = i * (v + 1);
        u32 p2 = p1 + (v + 1);
        for (u32 j = 0; j < v; j++, p1++, p2++)
        {
            result.indices[indices_count++] = p1;
            result.indices[indices_count++] = p1 + 1;
            result.indices[indices_count++] = p2 + 1;
            
            result.indices[indices_count++] = p1;
            result.indices[indices_count++] = p2 + 1;
            result.indices[indices_count++] = p2;
        }
    }
    opengl_init_mesh(&result);
    return result;
}

function Mesh
make_square_mesh_into_patches(Mesh *mesh, u32 u, u32 v)
{
    Mesh new_mesh = {};
    
    new_mesh.vertices_count = u * v * 4;
    new_mesh.vertices = (Vertex*)SDL_malloc(sizeof(Vertex) * new_mesh.vertices_count);
    
    u32 vertex_count = 0;
    for (u32 i = 0; i < u; i++)
    {
        u32 p1 = i * (v + 1);
        u32 p2 = p1 + (v + 1);
        for (u32 j = 0; j < v; j++, p1++, p2++)
        {
            new_mesh.vertices[vertex_count++] = mesh->vertices[p1];
            new_mesh.vertices[vertex_count++] = mesh->vertices[p1 + 1];
            new_mesh.vertices[vertex_count++] = mesh->vertices[p2];
            new_mesh.vertices[vertex_count++] = mesh->vertices[p2 + 1];
        }
    }
    
    opengl_init_mesh(&new_mesh);
    return new_mesh;
}

/*
function v3
apply_wave(v3 pos, v4 wave, f32 time)
{
    v2 direction = { wave.x, wave.y };
    f32 wave_length = wave.z;
    f32 steepness = wave.w;
    
    f32 k = 2 * PI / wave_length;
    f32 c = sqrt(9.8 / k);
    v2 d = normalized(direction);
    f32 f = k * (dot_product(d, { pos.x, pos.z }) - c * time);
    f32 a = steepness / k;
    
    return {
        d.x * (a * cosf(f)),
        a * sinf(f),
        d.y * (a * cosf(f))
    };
}
*/

global_variable Mesh rect = {};
global_variable Shader *draw_color_rect_2D_shader;
global_variable Shader *draw_color_rect_3D_shader;
global_variable Shader *draw_tex_rect_shader;
global_variable m4x4 orthographic_matrix;
global_variable m4x4 perspective_matrix;
global_variable m4x4 view_matrix;

inline void
draw_rect(v2 coords, v2 dim, v4 color)
{
    opengl_draw_rect(coords, dim, color, &rect, draw_color_rect_2D_shader, orthographic_matrix);
}

inline void
draw_rect(v3 coords, quat rotation, v3 dim, v4 color)
{
    opengl_draw_rect(coords, rotation, dim, color, &rect, draw_color_rect_3D_shader, perspective_matrix, view_matrix);
}

function void
do_one_frame(Application *app)
{
    Assets *assets = &app->assets;
    Storage *storage = &app->storage;
    Controller *controller = &app->controller;
    
    if (on_down(controller->pause))
        storage->paused = !storage->paused;
    
    if (!storage->paused)
    {
        app->relative_mouse_mode.set(TRUE);
        update_camera_with_mouse(&storage->camera, controller->mouse);
        
        f32 m_per_s = 3.0f;
        f32 move_speed = m_per_s * app->frame_time_s;
        update_camera_with_keys(&storage->camera,
                                {move_speed, move_speed, move_speed},
                                controller->forward,
                                controller->backward,
                                controller->left,
                                controller->right,
                                controller->up,
                                controller->down);
        
    }
    else if (storage->paused)
    {
        app->relative_mouse_mode.set(FALSE);
    }
    
    if (on_down(controller->reload_shaders))
    {
        reload_shader(&assets->shaders[0]);
        reload_shader(&assets->shaders[1]);
        reload_shader(&assets->shaders[2]);
    }
    
    //
    // Draw
    //
    
    r32 aspect_ratio = (r32)app->window_dim.Width / (r32)app->window_dim.Height;
    perspective_matrix = perspective_projection(90.0f, aspect_ratio, 0.01f, 1000.0f);
    orthographic_matrix = orthographic_projection(0.0f, 
                                                  (r32)app->window_dim.Width, (r32)app->window_dim.Height,
                                                  0.0f, -3.0f, 3.0f);
    view_matrix = get_view(storage->camera);
    
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
    
    if (on_down(controller->toggle_wireframe))
        app->wireframe.toggle();
    
    draw_rect(storage->ls_1.position, 
              get_rotation_to_direction(storage->camera.position - storage->ls_1.position, storage->camera.up),
              {0.25f, 0.25f, 0.25f}, {255, 255, 0, 1}); // Light
    
    draw_rect({0, 0, 0}, get_rotation(0, {1, 0, 0}), {1, 1, 1}, {255, 0, 0, 1}); // Red square
    
    // Water
    {
        u32 active_shader = use_shader(&assets->shaders[WATER]);
        v4 color = {166.0f/255.0f, 250.0f/255.0f, 255.0f/255.0f, 0.9};
        glUniform4fv(glGetUniformLocation(active_shader, "objectColor"), (GLsizei)1, (float*)&color);
        m4x4 model = create_transform_m4x4({0, 0, 0}, get_rotation(0, {1, 0, 0}), {20, 1, 20});
        glUniformMatrix4fv(glGetUniformLocation(active_shader, "model"), (GLsizei)1, false, (float*)&model);
        glUniformMatrix4fv(glGetUniformLocation(active_shader, "projection"), (GLsizei)1, false, (float*)&perspective_matrix);
        glUniformMatrix4fv(glGetUniformLocation(active_shader, "view"), (GLsizei)1, false, (float*)&view_matrix);
        glUniform1f(glGetUniformLocation(active_shader, "time"), app->run_time_s);
        glUniform3fv(glGetUniformLocation(active_shader, "lightPos"), (GLsizei)1, (float*)&storage->ls_1.position);
        glUniform3fv(glGetUniformLocation(active_shader, "lightColor"), (GLsizei)1, (float*)&storage->ls_1.color);
        glUniform3fv(glGetUniformLocation(active_shader, "cameraPos"), (GLsizei)1, (float*)&storage->camera.position);
        
        glBindVertexArray(storage->water.vao);
        glDrawArrays(GL_PATCHES, 0, storage->water.vertices_count);
        glBindVertexArray(0);
    }
    
    if (storage->paused)
    {
        glDisable(GL_DEPTH_TEST);
        glDisable(GL_CULL_FACE);
        
        draw_rect(v2{ 0, 0 }, cv2(app->window_dim), v4{ 0, 0, 0, 0.7 });
    }
    
    glUseProgram(0);
}

function void
load_assets(Assets *assets)
{
    assets->num_of_shaders = 4;
    assets->shaders = (Shader*)SDL_malloc(sizeof(Shader) * assets->num_of_shaders);
    
    assets->shaders[COLOR_RECT_2D] = load_shader("../assets/shaders/color2D.vs", 0, 0, 0, "../assets/shaders/color.fs");
    assets->shaders[TEXTURE_RECT_2D] = load_shader("../assets/shaders/color2D.vs", 0, 0, 0, "../assets/shaders/tex.fs");
    assets->shaders[COLOR_RECT_3D] = load_shader("../assets/shaders/color3D.vs", 0, 0, 0, "../assets/shaders/color.fs");
    assets->shaders[WATER] = load_shader("../assets/shaders/water.vs", 
                                         "../assets/shaders/water.tcs",
                                         "../assets/shaders/water.tes", 
                                         "../assets/shaders/water.gs", 
                                         "../assets/shaders/water.fs");
    
    //draw_color_rect_shader = &assets->shaders[COLOR_RECT_2D];
    draw_color_rect_2D_shader = get_shader(assets, COLOR_RECT_2D);
    draw_color_rect_3D_shader = get_shader(assets, COLOR_RECT_3D);
    draw_tex_rect_shader = &assets->shaders[TEXTURE_RECT_2D];
    
}

function void
init_storage(Storage* storage)
{
    storage->camera.position = {0, 0, 2};
    storage->camera.up = {0, 1, 0};
    storage->camera.target = {0, 0, -2};
    storage->camera.yaw = -90.0f;
    
    storage->ls_1.position = {0.0f, 5.0f, 0.0f};
    storage->ls_1.color = {1, 1, 1};
    
    create_rect_mesh(&storage->rect);
    
    Mesh temp_square_mesh = create_square_mesh(10, 10);
    Mesh temp_patch_mesh = make_square_mesh_into_patches(&temp_square_mesh, 10, 10);
    storage->water = temp_patch_mesh;
    
    create_rect_mesh(&rect);
}

function void
init_controller_buttons(Controller *controller)
{
    controller->right.id = KEY_D;
    controller->forward.id = KEY_W;
    controller->left.id = KEY_A;
    controller->backward.id = KEY_D;
    controller->up.id = KEY_SPACE;
    controller->down.id = KEY_LSHIFT;
    controller->pause.id = KEY_ESCAPE;
    controller->reload_shaders.id = KEY_R;
    controller->toggle_wireframe.id = KEY_T;
}

function void
init_app(Application *app)
{
    init_storage(&app->storage);
    load_assets(&app->assets);
    init_controller_buttons(&app->controller);
    
    // Default OpenGL Settings
    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_BLEND);
    glEnable(GL_DEPTH_TEST);
    glDepthMask(GL_TRUE);
    glEnable(GL_CULL_FACE);
    glPointSize(5.0f);
    glPatchParameteri(GL_PATCH_VERTICES, 4);
    
    glEnable(GL_DEBUG_OUTPUT);
    glDebugMessageCallback(opengl_debug_message_callback, 0);
    
    glViewport(0, 0, app->window_dim.Width, app->window_dim.Height);
    
    app->relative_mouse_mode.set(TRUE);
}

function void
main_loop(SDL_Window *window)
{
    Application app = {};
    SDL_SetRelativeMouseMode(SDL_TRUE);
    SDL_GetWindowSize(window, &app.window_dim.Width, &app.window_dim.Height);
    init_app(&app);
    
    u32 last_frame_run_time_ms = 0;
    while(1)
    {
        Controller *controller = &app.controller;
        controller->mouse = {};
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
        
        u32 run_time_ms = SDL_GetTicks();
        app.run_time_s = (f32)run_time_ms / 1000.0f;
        u32 frame_time_ms = run_time_ms - last_frame_run_time_ms;
        app.frame_time_s = (f32)frame_time_ms / 1000.0f;
        last_frame_run_time_ms = run_time_ms;
        
        
        do_one_frame(&app);
        
        // Relative Mouse Mode
        if (app.relative_mouse_mode.changed())
        {
            if (app.relative_mouse_mode.get())
                SDL_SetRelativeMouseMode(SDL_TRUE);
            else
                SDL_SetRelativeMouseMode(SDL_FALSE);
        }
        
        // Wireframe
        if (app.wireframe.changed())
        {
            if (app.wireframe.get())
                glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
            else
                glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        }
        
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