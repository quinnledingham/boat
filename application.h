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
            Button toggle_wireframe;
        };
        Button buttons[9];
    };
};

enum Asset_Tags
{
    COLOR_RECT_2D,
    COLOR_RECT_3D,
    TEXTURE_RECT_2D,
    WATER,
};

struct Storage
{
    Mesh rect;
    Mesh water;
    Camera camera;
    Light_Source ls_1;
    b32 paused;
};

struct Application
{
    v2s window_dim;
    Controller controller;
    Storage storage;
    Assets assets;
    
    Bool wireframe;
    Bool relative_mouse_mode;
    
    f32 run_time_s;
    f32 frame_time_s;
};

#endif //APPLICATION_H
