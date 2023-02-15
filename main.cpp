#include <stdio.h>

#include <glad.h>
#include <glad.c>
#include <SDL.h>

#include "types.h"
#include "log.h"
#include "math.h"
#include "rend.h"
#include "input.h"
#include "application.h"

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
    opengl_setup_mesh(&result);
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
    
    opengl_setup_mesh(&new_mesh);
    return new_mesh;
}

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
    
    storage->water_shader.vs_filename = "../data/shaders/water.vs";
    storage->water_shader.tcs_filename = "../data/shaders/water.tcs";
    storage->water_shader.tes_filename = "../data/shaders/water.tes";
    storage->water_shader.gs_filename = "../data/shaders/water.gs";
    storage->water_shader.fs_filename = "../data/shaders/water.fs";
    load_opengl_shader(&storage->water_shader);
    
    storage->ls_1.position = {0.0f, 5.0f, 0.0f};
    storage->ls_1.color = {1, 1, 1};
    
    init_rect_mesh(&storage->rect);
    
    Mesh temp_square_mesh = create_square_mesh(10, 10);
    Mesh temp_patch_mesh = make_square_mesh_into_patches(&temp_square_mesh, 10, 10);
    //storage->water = temp_square_mesh;
    storage->water = temp_patch_mesh;
    //free_mesh(&temp_square_mesh);
    //free_mesh(&temp_patch_mesh);
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
        
        f32 m_per_s = 3.0f;
        f32 move_speed = m_per_s * app->s_elapsed_frame;
        update_camera_with_keys(&storage->camera,
                                {move_speed, move_speed, move_speed},
                                controller->forward,
                                controller->backward,
                                controller->left,
                                controller->right,
                                controller->up,
                                controller->down);
        
    }
    else if (app->paused)
    {
        SDL_SetRelativeMouseMode(SDL_FALSE);
    }
    
    if (on_down(controller->reload_shaders))
    {
        load_opengl_shader(&storage->color_2D);
        load_opengl_shader(&storage->color_3D);
        load_opengl_shader(&storage->water_shader);
    }
    
    //
    // Draw
    //
    
    r32 aspect_ratio = (r32)app->window_dim.Width / (r32)app->window_dim.Height;
    m4x4 perspective_matrix = perspective_projection(90.0f, aspect_ratio, 0.01f, 1000.0f);
    m4x4 orthographic_matrix = orthographic_projection(0.0f, 
                                                       (r32)app->window_dim.Width, (r32)app->window_dim.Height,
                                                       0.0f, -3.0f, 3.0f);
    m4x4 view_matrix = get_view(storage->camera);
    
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
    
    if (on_down(controller->toggle_wireframe))
    {
        app->wireframe = !app->wireframe;
        
        if (app->wireframe)
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        else if (!app->wireframe)
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    }
    
    // Light
    {
        u32 active_shader = use_shader(&storage->color_3D);
        v4 color = {255, 255, 0, 1};
        glUniform4fv(glGetUniformLocation(active_shader, "user_color"), (GLsizei)1, (float*)&color);
        m4x4 model = create_transform_m4x4(storage->ls_1.position, 
                                           get_rotation_to_direction(storage->camera.position - storage->ls_1.position, storage->camera.up),
                                           {0.25f, 0.25f, 0.25f});
        glUniformMatrix4fv(glGetUniformLocation(active_shader, "model"), (GLsizei)1, false, (float*)&model);
        glUniformMatrix4fv(glGetUniformLocation(active_shader, "projection"), (GLsizei)1, false, (float*)&perspective_matrix);
        glUniformMatrix4fv(glGetUniformLocation(active_shader, "view"), (GLsizei)1, false, (float*)&view_matrix);
        opengl_draw_mesh(&storage->rect);
    }
    
    {
        u32 active_shader = use_shader(&storage->color_3D);
        v4 color = {255, 0, 0, 1};
        glUniform4fv(glGetUniformLocation(active_shader, "user_color"), (GLsizei)1, (float*)&color);
        m4x4 model = create_transform_m4x4({0, 0, 0}, get_rotation(0, {1, 0, 0}), {1, 1, 1});
        glUniformMatrix4fv(glGetUniformLocation(active_shader, "model"), (GLsizei)1, false, (float*)&model);
        glUniformMatrix4fv(glGetUniformLocation(active_shader, "projection"), (GLsizei)1, false, (float*)&perspective_matrix);
        glUniformMatrix4fv(glGetUniformLocation(active_shader, "view"), (GLsizei)1, false, (float*)&view_matrix);
        opengl_draw_mesh(&storage->rect);
    }
    
    {
        u32 active_shader = use_shader(&storage->water_shader);
        v4 color = {166.0f/255.0f, 250.0f/255.0f, 255.0f/255.0f, 0.9};
        glUniform4fv(glGetUniformLocation(active_shader, "objectColor"), (GLsizei)1, (float*)&color);
        m4x4 model = create_transform_m4x4({0, 0, 0}, get_rotation(0, {1, 0, 0}), {20, 1, 20});
        glUniformMatrix4fv(glGetUniformLocation(active_shader, "model"), (GLsizei)1, false, (float*)&model);
        glUniformMatrix4fv(glGetUniformLocation(active_shader, "projection"), (GLsizei)1, false, (float*)&perspective_matrix);
        glUniformMatrix4fv(glGetUniformLocation(active_shader, "view"), (GLsizei)1, false, (float*)&view_matrix);
        glUniform1f(glGetUniformLocation(active_shader, "time"), app->s_elapsed);
        glUniform3fv(glGetUniformLocation(active_shader, "lightPos"), (GLsizei)1, (float*)&storage->ls_1.position);
        glUniform3fv(glGetUniformLocation(active_shader, "lightColor"), (GLsizei)1, (float*)&storage->ls_1.color);
        glUniform3fv(glGetUniformLocation(active_shader, "cameraPos"), (GLsizei)1, (float*)&storage->camera.position);
        
        glBindVertexArray(storage->water.vao);
        glDrawArrays(GL_PATCHES, 0, storage->water.vertices_count);
        glBindVertexArray(0);
    }
    
    /*
    {
        u32 active_shader = use_shader(&storage->color_3D);
        v4 color = {0, 255, 0, 1};
        glUniform4fv(glGetUniformLocation(active_shader, "user_color"), (GLsizei)1, (float*)&color);
        m4x4 model = create_transform_m4x4({0, 0, 0}, {}, {1, 1, 1});
        glUniformMatrix4fv(glGetUniformLocation(active_shader, "model"), (GLsizei)1, false, (float*)&model);
        glUniformMatrix4fv(glGetUniformLocation(active_shader, "projection"), (GLsizei)1, false, (float*)&perspective_matrix);
        glUniformMatrix4fv(glGetUniformLocation(active_shader, "view"), (GLsizei)1, false, (float*)&view_matrix);
        //opengl_draw_mesh(&storage->water);
        glBindVertexArray(storage->water.vao);
        glDrawArrays(GL_POINTS, 0, storage->water.vertices_count);
        glBindVertexArray(0);
    }
    */
    if (app->paused)
    {
        glDisable(GL_DEPTH_TEST);
        glDisable(GL_CULL_FACE);
        u32 active_shader = use_shader(&storage->color_2D);
        v4 color = {0, 0, 0, 0.7};
        m4x4 model = create_transform_m4x4({0, 0, 0}, get_rotation(0, {1, 0, 0}), {(f32)app->window_dim.Width, (f32)app->window_dim.Height, 1});
        glUniform4fv(glGetUniformLocation(active_shader, "user_color"), (GLsizei)1, (float*)&color);
        glUniformMatrix4fv(glGetUniformLocation(active_shader, "model"), (GLsizei)1, false, (float*)&model);
        glUniformMatrix4fv(glGetUniformLocation(active_shader, "projection"), (GLsizei)1, false, (float*)&orthographic_matrix);
        opengl_draw_mesh(&storage->rect);
    }
    
    glUseProgram(0);
    
    //
    // End of Draw
    //
}

function void
main_loop(SDL_Window *window)
{
    Application app = {};
    SDL_GetWindowSize(window, &app.window_dim.Width, &app.window_dim.Height);
    SDL_SetRelativeMouseMode(SDL_TRUE);
    
    Controller *controller = &app.controller;
    controller->right.id = SDLK_d;
    controller->forward.id = SDLK_w;
    controller->left.id = SDLK_a;
    controller->backward.id = SDLK_s;
    controller->up.id = SDLK_SPACE;
    controller->down.id = SDLK_LSHIFT;
    controller->pause.id = SDLK_ESCAPE;
    controller->reload_shaders.id = SDLK_r;
    controller->toggle_wireframe.id = SDLK_t;
    
    uint32 last_ms_elapsed = 0;
    
    initialize_storage(&app.storage);
    
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
        
        uint32 ms_elapsed = SDL_GetTicks();
        app.s_elapsed = (f32)ms_elapsed / 1000.0f;
        uint32 ms_elapsed_frame = ms_elapsed - last_ms_elapsed;
        app.s_elapsed_frame = (f32)ms_elapsed_frame / 1000.0f;
        last_ms_elapsed = ms_elapsed;
        
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
    glEnable(GL_DEPTH_TEST);
    glDepthMask(GL_TRUE);
    glEnable(GL_CULL_FACE);
    glPointSize(5.0f);
    glPatchParameteri(GL_PATCH_VERTICES, 4);
    
    glEnable(GL_DEBUG_OUTPUT);
    glDebugMessageCallback(opengl_debug_message_callback, 0);
    
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