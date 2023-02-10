void GLAPIENTRY
opengl_debug_message_callback(GLenum source, GLenum type, GLuint id, GLenum severity,
                              GLsizei length, const GLchar* message, const void* userParam )
{
    SDL_Log("GL CALLBACK:");
    SDL_Log("message: %s\n", message);
    switch (type)
    {
        case GL_DEBUG_TYPE_ERROR:
        SDL_Log("type: ERROR");
        break;
        case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR:
        SDL_Log("type: DEPRECATED_BEHAVIOR");
        break;
        case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:
        SDL_Log("type: UNDEFINED_BEHAVIOR");
        break;
        case GL_DEBUG_TYPE_PORTABILITY:
        SDL_Log("type: PORTABILITY");
        break;
        case GL_DEBUG_TYPE_PERFORMANCE:
        SDL_Log("type: PERFORMANCE");
        break;
        case GL_DEBUG_TYPE_OTHER:
        SDL_Log("type: OTHER");
        break;
    }
    SDL_Log("id: %d", id);
    switch(severity)
    {
        case GL_DEBUG_SEVERITY_LOW:
        SDL_Log("severity: LOW\n");
        break;
        case GL_DEBUG_SEVERITY_MEDIUM:
        SDL_Log("severity: MEDIUM\n");
        break;
        case GL_DEBUG_SEVERITY_HIGH:
        SDL_Log("severity: HIGH\n");
        break;
    }
}

function void
debug_opengl_shader(u32 id)
{
    GLint length;
    glGetShaderiv(id, GL_INFO_LOG_LENGTH, &length);
    if (length > 0)
    {
        GLchar infoLog[512];
        GLint size; //gives 0 when checked in debugger
        glGetShaderInfoLog(id, 512, &size, infoLog);
        SDL_Log("%s", infoLog);
    }
}

function void
debug_opengl_program(u32 id)
{
    GLint length;
    glGetProgramiv(id, GL_INFO_LOG_LENGTH, &length);
    if (length > 0)
    {
        GLchar infoLog[512];
        GLint size; //gives 0 when checked in debugger
        glGetProgramInfoLog(id, 512, &size, infoLog);
        SDL_Log("%s", infoLog);
    }
}

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
compile_opengl_shader(Shader* shader, int type)
{
    const char *file = 0;
    switch (type)
    {
        case GL_VERTEX_SHADER: file = load_shader_file(shader->vs_filename); break;
        case GL_TESS_CONTROL_SHADER: file = load_shader_file(shader->tcs_filename); break;
        case GL_TESS_EVALUATION_SHADER: file = load_shader_file(shader->tes_filename); break;
        case GL_GEOMETRY_SHADER: file = load_shader_file(shader->gs_filename); break;
        case GL_FRAGMENT_SHADER: file = load_shader_file(shader->fs_filename); break;
        default:
        {
            error("compile_opengl_shader() not a type of shader");
            return false;
        } break;
    }
    if (file == 0)
        return false;
    
    u32 s = glCreateShader((GLenum)type);
    glShaderSource(s, 1, &file, NULL);
    glCompileShader(s);
    
    GLint compiled_s = 0;
    glGetShaderiv(s, GL_COMPILE_STATUS, &compiled_s);
    if (!compiled_s)
        debug_opengl_shader(s);
    else
        glAttachShader(shader->handle, s);
    
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
    
    if (shader->vs_filename != 0) // Vertex Shader
    {
        SDL_Log("Loading shader %s\n", shader->vs_filename);
        if (!compile_opengl_shader(shader, GL_VERTEX_SHADER))
        {
            error("load_opengl_shader() compiling vertex shader failed");
            return;
        }
    }
    else
    {
        error("load_opengl_shader() vertex shader is required");
        return;
    }
    
    if (shader->tcs_filename != 0) // Tessellation Control Shader
    {
        if (!compile_opengl_shader(shader, GL_TESS_CONTROL_SHADER))
        {
            error("load_opengl_shader() compiling tessellation control shader failed");
            return;
        }
    }
    
    if (shader->tes_filename != 0) // Tessellation Evaluation Shader
    {
        if (!compile_opengl_shader(shader, GL_TESS_EVALUATION_SHADER))
        {
            error("load_opengl_shader() compiling tessellation evaluation shader failed");
            return;
        }
    }
    
    if (shader->gs_filename != 0) // Geometry Shader
    {
        if (!compile_opengl_shader(shader, GL_GEOMETRY_SHADER))
        {
            error("load_opengl_shader() compiling geometry shader failed");
            return;
        }
    }
    
    if (shader->fs_filename != 0) // Fragment Shader
    {
        if (!compile_opengl_shader(shader, GL_FRAGMENT_SHADER))
        {
            error("load_opengl_shader() compiling fragment shader failed");
            return;
        }
    }
    
    // Link
    glLinkProgram(shader->handle);
    GLint linked_program = 0;
    glGetProgramiv(shader->handle, GL_LINK_STATUS, &linked_program);
    if (!linked_program)
    {
        debug_opengl_program(shader->handle);
        error("load_opengl_shader() link failed");
        return;
    }
    
    shader->compiled = true;
}

function void
opengl_setup_mesh(Mesh *mesh)
{
    glGenVertexArrays(1, &mesh->vao);
    glGenBuffers(1, &mesh->vbo);
    glGenBuffers(1, &mesh->ebo);
    
    glBindVertexArray(mesh->vao);
    glBindBuffer(GL_ARRAY_BUFFER, mesh->vbo);
    glBufferData(GL_ARRAY_BUFFER, mesh->vertices_count * sizeof(Vertex), &mesh->vertices[0], GL_STATIC_DRAW);  
    
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh->ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, mesh->indices_count * sizeof(u32), &mesh->indices[0], GL_STATIC_DRAW);
    
    // vertex positions
    glEnableVertexAttribArray(0);	
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
    // vertex normals
    glEnableVertexAttribArray(1);	
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, normal));
    // vertex texture coords
    glEnableVertexAttribArray(2);	
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
init_rect_mesh(Mesh *rect)
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
    
    opengl_setup_mesh(rect);
}

function inline m4x4
get_view(Camera camera)
{
    return look_at(camera.position, camera.position + camera.target, camera.up);
}