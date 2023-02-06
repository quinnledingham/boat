#ifndef APPLICATION_H
#define APPLICATION_H

// is the button down
// did it go down between this frame and the last
// is the button up
// did it go up between this frame and the last
struct Button
{
    int id; // what button is tied to the button
    b32 current_state; // down == true, up == false
    b32 previous_state;
}; // Toggle functions below

struct Controller
{
    v2 mouse;
    
    union
    {
        struct
        {
            Button right;
            Button up;
            Button left;
            Button down;
        };
        Button buttons[4];
    };
};

struct Storage
{
    Mesh rect;
    Shader color_shader;
    Camera camera;
};

struct Application
{
    v2s window_dim;
    Controller controller;
    Storage storage;
    
    b32 initialized;
};

//
// Button Toggle Functions
//

function b32
on_down(Button button)
{
    if (button.current_state && button.current_state != button.previous_state)
        return true;
    else
        return false;
}

function b32
is_down(Button button)
{
    if (button.current_state)
        return true;
    else
        return false;
}

#endif //APPLICATION_H
