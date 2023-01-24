#ifndef REND_H
#define REND_H

struct Shader_Variable
{
    char name[20];
    u32 location;
}

struct Shader
{
    const char *vertex_shader_filename; //.vs
    const char *tessellation_control_shader_filename; //.tcs
    const char *tessellation_evaluation_shader_filename; //.tes
    const char *geometry_shader_filename; //.gs
    const char *fragment_shader_filename; //.fs
    
    b32 compiled;
    u32 handle;
    
    Shader_Variable attributes[10];
    Shader_Variable uniforms[10];
};

struct Mesh_Vertex
{
    v3 position;
    v3 normal;
    v2 texture_coordinate;
};

struct Mesh
{
    Mesh_Vertex *vertices;
    u32 vertices_count;
    
    u32 *indices;
    u32 indices_count;
    
    u32 VAO;
    u32 VBO;
    u32 EBO;
};

#endif //REND_H
