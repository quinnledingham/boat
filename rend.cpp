//
// OpenGL Debug
//

void GLAPIENTRY
opengl_debug_message_callback(GLenum source, GLenum type, GLuint id, GLenum severity,
                              GLsizei length, const GLchar* message, const void* userParam )
{
    SDL_Log("GL CALLBACK:");
    SDL_Log("message: %s\n", message);
    switch (type)
    {
        case GL_DEBUG_TYPE_ERROR: SDL_Log("type: ERROR"); break;
        case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR: SDL_Log("type: DEPRECATED_BEHAVIOR"); break;
        case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR: SDL_Log("type: UNDEFINED_BEHAVIOR"); break;
        case GL_DEBUG_TYPE_PORTABILITY: SDL_Log("type: PORTABILITY"); break;
        case GL_DEBUG_TYPE_PERFORMANCE: SDL_Log("type: PERFORMANCE"); break;
        case GL_DEBUG_TYPE_OTHER: SDL_Log("type: OTHER"); break;
    }
    SDL_Log("id: %d", id);
    switch(severity)
    {
        case GL_DEBUG_SEVERITY_LOW: SDL_Log("severity: LOW\n"); break;
        case GL_DEBUG_SEVERITY_MEDIUM: SDL_Log("severity: MEDIUM\n"); break;
        case GL_DEBUG_SEVERITY_HIGH: SDL_Log("severity: HIGH\n"); break;
    }
}

function void
debug_opengl(u32 type, u32 id)
{
    GLint length;
    glGetShaderiv(id, GL_INFO_LOG_LENGTH, &length);
    if (length > 0)
    {
        GLchar info_log[512];
        GLint size;
        
        switch(type)
        {
            case GL_SHADER: glGetShaderInfoLog(id, 512, &size, info_log); break;
            case GL_PROGRAM: glGetProgramInfoLog(id, 512, &size, info_log); break;
        }
        
        log(info_log);
    }
}

//
// Shader
//

function const char*
load_shader_file(const char* filename)
{
    FILE *file = fopen(filename, "rb");
    if (file == 0)
    {
        error("load_shader_file() could not open file");
        return 0;
    }
    
    fseek(file, 0, SEEK_END);
    u32 size = ftell(file);
    fseek(file, 0, SEEK_SET);
    
    char* shader_file = (char*)malloc(size + 1);
    fread(shader_file, size, 1, file);
    shader_file[size] = 0;
    fclose(file);
    
    return shader_file;
}

function bool
compile_opengl_shader(u32 handle, const char *filename, int type)
{
    const char *file = load_shader_file(filename);
    if (file == 0)
        return false;
    
    u32 s =  glCreateShader((GLenum)type);
    glShaderSource(s, 1, &file, NULL);
    glCompileShader(s);
    
    GLint compiled_s = 0;
    glGetShaderiv(s, GL_COMPILE_STATUS, &compiled_s);
    if (!compiled_s)
        debug_opengl(GL_SHADER, s);
    else
        glAttachShader(handle, s);
    
    glDeleteShader(s);
    free((void*)file);
    
    return compiled_s;
}

function void
load_opengl_shader(Shader *shader)
{
    shader->compiled = false;
    if (shader->handle != 0)
        glDeleteProgram(shader->handle);
    shader->handle = glCreateProgram();
    
    
    if (!compile_opengl_shader(shader->handle, shader->vs_filename, GL_VERTEX_SHADER))
    {
        printf("vs: %s\n", shader->vs_filename);
        error("load_opengl_shader() compiling vertex shader failed");
        return;
    }
    if (shader->tcs_filename != 0)
    {
        if (!compile_opengl_shader(shader->handle, shader->tcs_filename, GL_TESS_CONTROL_SHADER))
            error("load_opengl_shader() compiling tess control shader failed");
    }
    if (shader->tes_filename != 0)
    {
        if (!compile_opengl_shader(shader->handle, shader->tes_filename, GL_TESS_EVALUATION_SHADER))
            error("load_opengl_shader() compiling tess_evaluation shader failed");
    }
    if (shader->gs_filename != 0)
    {
        if (!compile_opengl_shader(shader->handle, shader->gs_filename, GL_GEOMETRY_SHADER))
            error("load_opengl_shader() compiling geometry shader failed");
    }
    if (shader->fs_filename != 0)
    {
        if (!compile_opengl_shader(shader->handle, shader->fs_filename, GL_FRAGMENT_SHADER))
            error("load_opengl_shader() compiling fragment shader failed");
    }
    
    // Link
    glLinkProgram(shader->handle);
    GLint linked_program = 0;
    glGetProgramiv(shader->handle, GL_LINK_STATUS, &linked_program);
    if (!linked_program)
    {
        debug_opengl(GL_PROGRAM, shader->handle);
        error("load_opengl_shader() link failed");
        return;
    }
    
    shader->compiled = true;
}

function Shader
load_shader(const char *vs_filename,
            const char *tcs_filename,
            const char *tes_filename,
            const char *gs_filename,
            const char *fs_filename)
{
    Shader shader = {};
    
    shader.vs_filename = copy(vs_filename);
    shader.tcs_filename = copy(tcs_filename);
    shader.tes_filename = copy(tes_filename);
    shader.gs_filename = copy(gs_filename);
    shader.fs_filename = copy(fs_filename);
    
    load_opengl_shader(&shader);
    
    return shader;
}

function void
reload_shader(Shader *shader)
{
    load_opengl_shader(shader);
}

//
// Mesh
//

function void
opengl_init_mesh(Mesh *mesh)
{
    glGenVertexArrays(1, &mesh->vao);
    glGenBuffers(1, &mesh->vbo);
    glGenBuffers(1, &mesh->ebo);
    
    glBindVertexArray(mesh->vao);
    glBindBuffer(GL_ARRAY_BUFFER, mesh->vbo);
    glBufferData(GL_ARRAY_BUFFER, mesh->vertices_count * sizeof(Vertex), &mesh->vertices[0], GL_STATIC_DRAW);  
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh->ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, mesh->indices_count * sizeof(u32), &mesh->indices[0], GL_STATIC_DRAW);
    
    glEnableVertexAttribArray(0); // vertex positions
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
    glEnableVertexAttribArray(1); // vertex normals
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, normal));
    glEnableVertexAttribArray(2); // vertex texture coords
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, texture_coordinate));
    
    glBindVertexArray(0);
}

function void
opengl_draw_mesh(Mesh *mesh)
{
    glBindVertexArray(mesh->vao);
    glDrawElements(GL_TRIANGLES, mesh->indices_count, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}

//
// Rect
//

function void
init_rect_indices(u32 *indices, 
                  u32 top_left, 
                  u32 top_right,
                  u32 bottom_left,
                  u32 bottom_right)
{
    indices[0] = top_left;
    indices[1] = bottom_left;
    indices[2] = bottom_right;
    indices[3] = top_left;
    indices[4] = bottom_right;
    indices[5] = top_right;
}

function void
create_rect_mesh(Mesh *rect)
{
    rect->vertices_count = 4;
    rect->vertices = (Vertex*)SDL_malloc(sizeof(Vertex) * rect->vertices_count);
    rect->vertices[0] = { {0, 0, 0}, {0, 0, 1}, {0, 0} };
    rect->vertices[1] = { {0, 1, 0}, {0, 0, 1}, {0, 0} };
    rect->vertices[2] = { {1, 0, 0}, {0, 0, 1}, {0, 0} };
    rect->vertices[3] = { {1, 1, 0}, {0, 0, 1}, {0, 0} };
    
    rect->indices_count = 6;
    rect->indices = (u32*)SDL_malloc(sizeof(u32) * rect->indices_count);
    init_rect_indices(rect->indices, 1, 3, 0, 2);
    
    opengl_init_mesh(rect);
}

function void
opengl_draw_rect(v2 coords, v2 dim, v4 color,
                 Mesh *rect, Shader *shader, m4x4 projection_matrix)
{
    u32 handle = use_shader(shader);
    glUniform4fv(glGetUniformLocation(handle, "user_color"), (GLsizei)1, (float*)&color);
    m4x4 model = create_transform_m4x4({coords.x, coords.y, 0}, get_rotation(0, {1, 0, 0}), {dim.x, dim.y, 1.0f});
    glUniformMatrix4fv(glGetUniformLocation(handle, "model"), (GLsizei)1, false, (float*)&model);
    glUniformMatrix4fv(glGetUniformLocation(handle, "projection"), (GLsizei)1, false, (float*)&projection_matrix);
    opengl_draw_mesh(rect);
}

function void
opengl_draw_rect(v3 coords, quat rotation, v3 dim, v4 color,
                 Mesh *rect, Shader *shader, m4x4 projection_matrix, m4x4 view_matrix)
{
    u32 handle = use_shader(shader);
    glUniform4fv(glGetUniformLocation(handle, "user_color"), (GLsizei)1, (float*)&color);
    m4x4 model = create_transform_m4x4(coords, rotation, dim);
    glUniformMatrix4fv(glGetUniformLocation(handle, "model"), (GLsizei)1, false, (float*)&model);
    glUniformMatrix4fv(glGetUniformLocation(handle, "projection"), (GLsizei)1, false, (float*)&projection_matrix);
    glUniformMatrix4fv(glGetUniformLocation(handle, "view"), (GLsizei)1, false, (float*)&view_matrix);
    opengl_draw_mesh(rect);
}

function void
opengl_draw_rect(v2 coords, v2 dim, Bitmap *bitmap,
                 Mesh *rect, Shader *shader, m4x4 projection_matrix)
{
    
    
    u32 handle = use_shader(shader);
    m4x4 model = create_transform_m4x4({coords.x, coords.y, 0}, get_rotation(0, {1, 0, 0}), {dim.x, dim.y, 1.0f});
    glUniformMatrix4fv(glGetUniformLocation(handle, "model"), (GLsizei)1, false, (float*)&model);
    glUniformMatrix4fv(glGetUniformLocation(handle, "projection"), (GLsizei)1, false, (float*)&projection_matrix);
    
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, bitmap->handle);
    glUniform1i(glGetUniformLocation(handle, "tex0"), 0);
    
    opengl_draw_mesh(rect);
}