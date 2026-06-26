#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <iostream>
#include <fstream>
#include <string>
#include <sstream>

const std::string shader_path = "res/shaders/basic.shader";

struct shader_prog_src {
    std::string vertex_src; 
    std::string fragment_src; 
};

static shader_prog_src parse_shader(const std::string& path) {
    enum class shader_type {
        NONE = -1, 
        VERTEX = 0, 
        FRAGMENT = 1
    }; 
    
    std::ifstream stream(path);
    
    std::string line; 
    std::stringstream sstream[2]; 
    shader_type type = shader_type::NONE;  

    while(getline(stream, line)) {
        if (line.find("#shader") != std::string::npos) {
            if (line.find("vertex") != std::string::npos) {
                type = shader_type::VERTEX;
            } else if (line.find("fragment") != std::string::npos) {
                type = shader_type::FRAGMENT; 
            } else {
                std::cerr << "unsupported shader type" << std::endl; 
                type = shader_type::NONE; 
            }
        } else if (type != shader_type::NONE) {
            sstream[(int)type] << line << '\n'; 
        }
    }

    return {sstream[0].str(), sstream[1].str()};
}

static uint32_t compile_shader(uint32_t type, const std::string& src) {
    uint32_t id = glCreateShader(type); 
    const char* src_raw = src.c_str(); 
    glShaderSource(id, 1, &src_raw, nullptr); 
    glCompileShader(id); 

    int res; 
    glGetShaderiv(id, GL_COMPILE_STATUS, &res);
    if (res == GL_FALSE) {
        int len; 
        glGetShaderiv(id, GL_INFO_LOG_LENGTH,  &len); 
        char* msg = (char*)alloca(len * sizeof(char)); 

        glGetShaderInfoLog(id, len, &len, msg); 
        std::cerr << (type == GL_VERTEX_SHADER ? "vertex " : "fragment ") << "shader compilation failed" << std::endl; 
        std::cerr << msg << std::endl; 
        glDeleteShader(id); 
        return 0; 
    }



    return id; 
}

static int check_program_err(uint32_t prog, int32_t status, const std::string& title) {
    int res; 

    glGetProgramiv(prog, status, &res); 
    if (res == GL_FALSE) {
        int len; 
        glGetProgramiv(prog, GL_INFO_LOG_LENGTH,  &len); 
        char* msg = (char*)alloca(len * sizeof(char)); 
        glGetProgramInfoLog(prog, len, &len, msg); 
        std::cerr << title << std::endl; 
        std::cerr << msg << std::endl; 

        glDeleteProgram(prog);

        return -1;  
    }
    return 0; 
}

static uint32_t create_shader(const std::string& vertex_shader, const std::string& fragment_shader) {
    uint32_t prog = glCreateProgram();
    uint32_t vs = compile_shader(GL_VERTEX_SHADER, vertex_shader); 
    if (!vs) {
        glDeleteProgram(prog); 
        return 0; 
    }
    uint32_t fs = compile_shader(GL_FRAGMENT_SHADER, fragment_shader); 
    if (!fs) {
        glDeleteShader(vs); 
        glDeleteProgram(prog); 
        return 0; 
    }
    

    glAttachShader(prog, vs); 
    uint32_t err = glGetError();
    if (err != GL_NO_ERROR) {
        std::cerr << "attatching vertex shader failed" << std::endl; 
        glDeleteShader(vs); 
        glDeleteShader(fs); 
        glDeleteProgram(prog);
        return 0; 
    }

    glAttachShader(prog, fs);
    err = glGetError();
    if (err != GL_NO_ERROR) {
        std::cerr << "attatching fragment shader failed" << std::endl; 
        glDeleteShader(vs); 
        glDeleteShader(fs); 
        glDeleteProgram(prog);
        return 0; 
    }

    int res; 

    glLinkProgram(prog);
    if (check_program_err(prog, GL_LINK_STATUS, "program linking failed") == -1) {
        glDeleteShader(vs); 
        glDeleteShader(fs); 
        return 0; 
    }
    
    glValidateProgram(prog);

    if (check_program_err(prog, GL_VALIDATE_STATUS, "program linking failed") == -1) {
        glDeleteShader(vs); 
        glDeleteShader(fs); 
        return 0; 
    }

    glDeleteShader(vs); 
    glDeleteShader(fs); 
    return prog; 
} 

void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
}


int main(int argc, char* argv[]) {
    //std::cout << "Hello OpenGL!" << std::endl; 

    if (!glfwInit()) {
        std::cerr << "glfw init failed" << std::endl;
        return 1; 
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4); 
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6); 
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(800, 600, "BlockGame", nullptr, nullptr); 
    if (!window) {
        std::cerr << "window init failed" << std::endl; 
        return 1; 
    }

    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback); 

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cerr << "glad init failed" << std::endl; 
        return 1; 
    }

    std::cout << "OpenGL: "
          << glGetString(GL_VERSION)
          << std::endl;

    float positions[] = {
        -0.5f, -0.5f, 
        0.5f, -0.5f, 
        0.5f, 0.5f, 
        -0.5f, 0.5f
    };

    uint32_t indices[] {
        0, 1, 2, 
        2, 3, 0
    };

    uint32_t vao;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);


    uint32_t buff; 
    glGenBuffers(1, &buff); 
    glBindBuffer(GL_ARRAY_BUFFER, buff); 
    glBufferData(GL_ARRAY_BUFFER, 8 * sizeof(float), positions, GL_STATIC_DRAW);

    glEnableVertexAttribArray(0); 
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), 0);  

    uint32_t ibo; 
    glGenBuffers(1, &ibo); 
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo); 
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, 6 * sizeof(uint32_t), indices, GL_STATIC_DRAW);

    shader_prog_src src = parse_shader(shader_path);
    std::cout << src.vertex_src << src.fragment_src << std::endl; 
    uint32_t shader = create_shader(src.vertex_src, src.fragment_src); 
    glUseProgram(shader); 

    while (!glfwWindowShouldClose(window)) {
        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
            glfwSetWindowShouldClose(window, true);
        }

        

        glClearColor(0.1f, 0.1f, 0.15f, 1.0f); 
        glClear(GL_COLOR_BUFFER_BIT); 

        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr); 

        glfwSwapBuffers(window); 
        glfwPollEvents(); 
    }

    glDeleteProgram(shader); 

    return 0; 
}