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
            Button pause;
        };
        Button buttons[5];
    };
};

struct Storage
{
    Mesh rect;
    Shader color_2D;
    Shader color_3D;
    Camera camera;
};

struct Application
{
    v2s window_dim;
    Controller controller;
    Storage storage;
    b32 paused;
    
    b32 initialized;
};

#endif //APPLICATION_H
