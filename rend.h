#ifndef REND_H
#define REND_H

struct Camera
{
    v3 position;
    v3 target;
    v3 up;
    real32 fov;
    real32 yaw;
    real32 pitch;
};
function inline m4x4 get_view(Camera camera) { return look_at(camera.position, camera.position + camera.target, camera.up); }

struct Light_Source
{
    v3 position;
    v3 color;
};

struct Shader
{
    const char *vs_filename; //.vs vertex_shader
    const char *tcs_filename; //.tcs tessellation control shader
    const char *tes_filename; //.tes tessellation evaluation shader
    const char *gs_filename; //.gs geometry shader
    const char *fs_filename; //.fs fragment shader
    
    b32 compiled;
    u32 handle;
};

inline u32
use_shader(Shader *shader)
{
    glUseProgram(shader->handle);
    return shader->handle;
}

struct Vertex
{
    v3 position;
    v3 normal;
    v2 texture_coordinate;
};

struct Mesh
{
    Vertex *vertices;
    u32 vertices_count;
    
    u32 *indices;
    u32 indices_count;
    
    u32 vao;
    u32 vbo;
    u32 ebo;
};

//
// Rect
//



#endif //REND_H
