#ifndef APPLICATION_H
#define APPLICATION_H

struct Controller
{
    v2s mouse;
    
    union
    {
        struct
        {
            Button right;
            Button forward;
            Button left;
            Button backward;
            Button up;
            Button down;
            Button pause;
            Button reload_shaders;
        };
        Button buttons[8];
    };
};

struct Storage
{
    Mesh rect;
    Mesh water;
    Shader color_2D;
    Shader color_3D;
    Shader water_shader;
    Camera camera;
    Light_Source ls_1;
};

struct Application
{
    v2s window_dim;
    Controller controller;
    Storage storage;
    
    b32 paused;
    f32 s_elapsed_frame;
    
    b32 initialized;
};

#endif //APPLICATION_H
