
//========================================================================
// Simple GLFW example
// Copyright (c) Camilla Löwy <elmindreda@glfw.org>
//
// This software is provided 'as-is', without any express or implied
// warranty. In no event will the authors be held liable for any damages
// arising from the use of this software.
//
// Permission is granted to anyone to use this software for any purpose,
// including commercial applications, and to alter it and redistribute it
// freely, subject to the following restrictions:
//
// 1. The origin of this software must not be misrepresented; you must not
//    claim that you wrote the original software. If you use this software
//    in a product, an acknowledgment in the product documentation would
//    be appreciated but is not required.
//
// 2. Altered source versions must be plainly marked as such, and must not
//    be misrepresented as being the original software.
//
// 3. This notice may not be removed or altered from any source
//    distribution.
//
//========================================================================
//! [code]

#include <iostream>

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "linmath.h"

#include <stdlib.h>
#include <stdio.h>

static const struct
{
    float x, y;
    float r, g, b;
} vertices[3] =
{
    { -0.6f, -0.4f, 1.f, 0.f, 0.f },
    {  0.6f, -0.4f, 0.f, 1.f, 0.f },
    {   0.f,  0.6f, 0.f, 0.f, 1.f }
};

static const char* vertex_shader_text =
"#version 110\n"
"uniform mat4 MVP;\n"
"attribute vec3 vCol;\n"
"attribute vec2 vPos;\n"
"varying vec3 color;\n"
"void main()\n"
"{\n"
"    gl_Position = MVP * vec4(vPos, 0.0, 1.0);\n"
"    color = vCol;\n"
"}\n";

static const char* fragment_shader_text =
"#version 110\n"
"varying vec3 color;\n"
"void main()\n"
"{\n"
"    gl_FragColor = vec4(color, 1.0);\n"
"}\n";

///Drawing Sprites //
static const char* vertex_shader_sprite_text =
        "#version 330 core\n"
        "layout (location = 0) in vec4 vertex;\n"
        "out vec2 TexCoords;\n"
        "uniform mat4 model;\n"
        "uniform mat4 projection;\n"
        "void main()\n"
        "{\n"
           " TexCoords = vertex.zw;\n"
           " gl_Position = projection * model * vec4(vertex.xy, 0.0, 1.0);\n"
        "}\n";

static const char* fragment_shader_sprite_text =
"#version 330 core\n"
        "in vec2 TexCoords;\n"
        "out vec4 color;\n"
        "uniform sampler2D image;\n"
        "uniform vec3 spriteColor;\n"
        "void main()\n"
        "{\n"
            "color = vec4(spriteColor, 1.0) * texture(image, TexCoords);\n"
        "}\n";

static const char* fragment_shader_particle_text =
        "layout (location = 0) in vec4 vertex; \n"
        "out vec2 TexCoords;\n"
        "out vec4 ParticleColor;\n"
        "uniform mat4 projection;\n"
        "uniform vec2 offset;\n"
        "uniform vec4 color;\n"
        "void main()\n"
        "{\n"
        "   float scale = 10.0f;\n"
        "   TexCoords = vertex.zw;\n"
        "    ParticleColor = color;\n"
        "    gl_Position = projection * vec4((vertex.xy * scale) + offset, 0.0, 1.0);\n"
        "}\n";

static const char* vertex_shader_particle_text =
        "#version 330 core\n"
        "layout (location = 0) in vec4 vertex;\n"
        "out vec2 TexCoords;\n"
        "out vec4 ParticleColor;\n"
         "uniform mat4 projection;\n"
        "uniform vec2 offset;\n"
        "uniform vec4 color;\n"
        "void main()\n"
        "{\n"
        "float scale = 10.0f;\n"
        "TexCoords = vertex.zw;\n"
        "ParticleColor = color;\n"
        "gl_Position = projection * vec4((vertex.xy * scale) + offset, 0.0, 1.0);\n"
        "}\n";



static void error_callback(int error, const char* description)
{
    fprintf(stderr, "Error: %s\n", description);
}

static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GLFW_TRUE);
}

int main(void)
{
    int  success;
    char infoLog[512];

    GLFWwindow* window;
    GLuint vertex_buffer, vertex_shader, fragment_shader, program;
    GLuint vertex_shader_sprite, fragment_shader_sprite,program_sprite;
    GLuint vertex_shader_particle, fragment_shader_particle,program_particle;

    GLint mvp_location, vpos_location, vcol_location;
    GLint model_location, projection_location; //For Sprite

    glfwSetErrorCallback(error_callback);

    if (!glfwInit())
        exit(EXIT_FAILURE);

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);

    const int WIN_WIDTH = 640;
    const int WIN_HEIGHT = 480;
    window = glfwCreateWindow(640, 480, "Simple example", NULL, NULL);
    if (!window)
    {
        glfwTerminate();
        exit(EXIT_FAILURE);
    }

    glfwSetKeyCallback(window, key_callback);

    glfwMakeContextCurrent(window);
    gladLoadGLLoader((GLADloadproc) glfwGetProcAddress);
    glfwSwapInterval(1);

    // NOTE: OpenGL error checks have been omitted for brevity

    glGenBuffers(1, &vertex_buffer);
    glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    vertex_shader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertex_shader, 1, &vertex_shader_text, NULL);
    glCompileShader(vertex_shader);

    fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragment_shader, 1, &fragment_shader_text, NULL);
    glCompileShader(fragment_shader);

    program = glCreateProgram();
    glAttachShader(program, vertex_shader);
    glAttachShader(program, fragment_shader);

    glLinkProgram(program);

    mvp_location = glGetUniformLocation(program, "MVP");
    vpos_location = glGetAttribLocation(program, "vPos");
    vcol_location = glGetAttribLocation(program, "vCol");

    glEnableVertexAttribArray(vpos_location);
    glVertexAttribPointer(vpos_location, 2, GL_FLOAT, GL_FALSE,
                          sizeof(vertices[0]), (void*) 0);
    glEnableVertexAttribArray(vcol_location);
    glVertexAttribPointer(vcol_location, 3, GL_FLOAT, GL_FALSE,
                          sizeof(vertices[0]), (void*) (sizeof(float) * 2));

 ///////////////////////////////////////////
    /// ~~~  Sprite Shaders ~~~ ///////////
    vertex_shader_sprite = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertex_shader_sprite, 1, &vertex_shader_sprite_text, NULL);
    glCompileShader(vertex_shader_sprite);
    glGetShaderiv(vertex_shader_sprite, GL_COMPILE_STATUS, &success);
    if(!success)
    {
        glGetShaderInfoLog(vertex_shader_sprite, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
    }

    fragment_shader_sprite = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragment_shader_sprite, 1, &fragment_shader_sprite_text, NULL);
    glCompileShader(fragment_shader_sprite);
    glGetShaderiv(fragment_shader_sprite, GL_COMPILE_STATUS, &success);
    if(!success)
    {
        glGetShaderInfoLog(fragment_shader_sprite, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
    }

    program_sprite = glCreateProgram();
    glAttachShader(program_sprite, vertex_shader_sprite);
    glAttachShader(program_sprite, fragment_shader_sprite);
    glLinkProgram(program_sprite);

    // Get GPU mem Location For Parameters //
    model_location = glGetUniformLocation(program_sprite, "model");
    projection_location = glGetUniformLocation(program_sprite, "projection");

    //Set Sprite Shader Params
    glm::mat4 model;
     // First translate (transformations are: scale happens first, then rotation and then finall translation happens; reversed order)
     glm::vec2 position = glm::vec2(200, 200);
     glm::vec2 size = glm::vec2(300, 400);
     GLfloat rotate = 45.0f;
     glm::vec3 color = glm::vec3(0.0f, 1.0f, 0.0f);

    model = glm::translate(model, glm::vec3(position, 0.0f));
    model = glm::translate(model, glm::vec3(0.5f * size.x, 0.5f * size.y, 0.0f)); // Move origin of rotation to center of quad
    model = glm::rotate(model, rotate, glm::vec3(0.0f, 0.0f, 1.0f)); // Then rotate
    model = glm::translate(model, glm::vec3(-0.5f * size.x, -0.5f * size.y, 0.0f)); // Move origin back
    model = glm::scale(model, glm::vec3(size, 1.0f)); // Last scale

    glm::mat4 projection = glm::ortho(0.0f, static_cast<GLfloat>(WIN_WIDTH), static_cast<GLfloat>(WIN_HEIGHT), 0.0f, -1.0f, 1.0f);



    while (!glfwWindowShouldClose(window))
    {
        float ratio;
        int width, height;
        mat4x4 m, p, mvp;

        glfwGetFramebufferSize(window, &width, &height);
        ratio = width / (float) height;

        glViewport(0, 0, width, height);
        glClear(GL_COLOR_BUFFER_BIT);

        mat4x4_identity(m);
        mat4x4_rotate_Z(m, m, (float) glfwGetTime());
        mat4x4_ortho(p, -ratio, ratio, -1.f, 1.f, 1.f, -1.f);
        mat4x4_mul(mvp, p, m);

        glUseProgram(program);
        glUniformMatrix4fv(mvp_location, 1, GL_FALSE, (const GLfloat*) mvp);
        glDrawArrays(GL_TRIANGLES, 0, 3);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwDestroyWindow(window);

    glfwTerminate();
    exit(EXIT_SUCCESS);
}

//! [code]
