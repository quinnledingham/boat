#ifndef REND_H
#define REND_H

struct Shader_Variable
{
    char name[20];
    u32 location;
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
    
    Shader_Variable attributes[10];
    Shader_Variable uniforms[10];
};

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

#endif //REND_H
