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

static uint32_t create_shader(const std::string& vertex_shader, const std::string& fragment_shader) {
    uint32_t prog = glCreateProgram();
    uint32_t vs = compile_shader(GL_VERTEX_SHADER, vertex_shader); 
    uint32_t fs = compile_shader(GL_FRAGMENT_SHADER, fragment_shader); 

    glAttachShader(prog, vs); 
    glAttachShader(prog, fs);
    glLinkProgram(prog); 
    glValidateProgram(prog); 

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

    float positions[6] = {
        -0.5f, -0.5f, 
        0.0f, 0.5f, 
        0.5f, -0.5f
    };

    uint32_t vao;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);


    uint32_t buff; 
    glGenBuffers(1, &buff); 
    glBindBuffer(GL_ARRAY_BUFFER, buff); 
    glBufferData(GL_ARRAY_BUFFER, 6 * sizeof(float), positions, GL_STATIC_DRAW);

    glEnableVertexAttribArray(0); 
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), 0);  

    shader_prog_src src = parse_shader(shader_path);
    uint32_t shader = create_shader(src.vertex_src, src.fragment_src); 
    glUseProgram(shader); 

    while (!glfwWindowShouldClose(window)) {
        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
            glfwSetWindowShouldClose(window, true);
        }

        

        glClearColor(0.1f, 0.1f, 0.15f, 1.0f); 
        glClear(GL_COLOR_BUFFER_BIT); 

        glDrawArrays(GL_TRIANGLES, 0, 3); 

        glfwSwapBuffers(window); 
        glfwPollEvents(); 
    }

    glDeleteProgram(shader); 

    return 0; 
}