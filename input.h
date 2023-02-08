#ifndef INPUT_H
#define INPUT_H

// is the button down
// did it go down between this frame and the last
// is the button up
// did it go up between this frame and the last
struct Button
{
    int id; // what button is tied to the button
    b32 current_state; // down == true, up == false
    b32 previous_state;
};

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

// delta_mouse is a relative mouse movement amount
// as opposed to the screen coords of the mouse
function void
update_camera_with_mouse(Camera *camera, v2s delta_mouse)
{
    if (delta_mouse.x > 1 || delta_mouse.x < -1)
        camera->yaw += (f32)delta_mouse.x * 0.1f;
    if (delta_mouse.y > 1 || delta_mouse.y < -1)
        camera->pitch -= (f32)delta_mouse.y * 0.1f;
    
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
update_camera_with_keys(Camera *camera,
                        v3 move_vector,
                        Button forward,
                        Button backward,
                        Button left,
                        Button right,
                        Button up,
                        Button down)
{
    if (is_down(forward))
        camera->position += camera->target * move_vector;
    if (is_down(backward))
        camera->position -= camera->target * move_vector;
    if (is_down(left))
        camera->position -= normalized(cross_product(camera->target, camera->up)) * move_vector;
    if (is_down(right))
        camera->position += normalized(cross_product(camera->target, camera->up)) * move_vector;
    if (is_down(up))
        camera->position.y += move_vector.y;
    if (is_down(down))
        camera->position.y -= move_vector.y;
}

#endif //INPUT_H
