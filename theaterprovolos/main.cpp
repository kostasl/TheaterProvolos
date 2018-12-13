/// \title theaterprovolos an augmented reality for a fun onstage perfomance
/// made for 1028 Xmas party as KCL, CDN Meyer Lab's.
/// It takes images from a camera, and augments them with graphics - We are trying to
/// show off our mutants superpowers - Its all science fiction until someone makes it a reality
///
///I used the breakout game as base from which to create OpenGL sprites and graphics
/// OpenCV is used to obtain and track an actors motion
/// \author Kostas Lagogiannis 2018


/*******************************************************************
/// Using the breakout game example combined with opencv to project graphics features
/// to locations observed by camera in realtime
///
** This code is part of Breakout.
**
** Breakout is free software: you can redistribute it and/or modify
** it under the terms of the CC BY 4.0 license as published by
** Creative Commons, either version 4 of the License, or (at your
** option) any later version.
**
**
**
******************************************************************/
/// \notes Then it should be a matter of rendering some rectangle or a box (or whatever you wish to locate using OpenCV) -
/// there are plenty of tutorials for that, and then copying the rendered data using the glReadPixels()
/// function to client memory and passing that to OpenCV
///
///
#include <QCoreApplication>


#define GLEW_STATIC
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "game.h"
#include "resource_manager.h"


// GLFW function declerations
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode);

// The Width of the screen
const GLuint SCREEN_WIDTH = 800;
// The height of the screen
const GLuint SCREEN_HEIGHT = 600;

Game Breakout (SCREEN_WIDTH, SCREEN_HEIGHT);



int main(int argc, char *argv[])
{



    QCoreApplication a(argc, argv);

    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);

    GLFWwindow* window = glfwCreateWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Breakout", nullptr, nullptr);
    glfwMakeContextCurrent(window);

    //glfwSwapInterval(1); //KL

    glewExperimental = GL_TRUE;
    glewInit();
    glGetError(); // Call it once to catch glewInit() bug, all other errors are now from our application.

    glfwSetKeyCallback(window, key_callback);

    // OpenGL configuration
    glViewport(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);

    //glEnable(GL_CULL_FACE);
    //glEnable(GL_BLEND);
    //glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Initialize game
    Breakout.Init();

    // DeltaTime variables
    GLfloat deltaTime = 0.0f;
    GLfloat lastFrame = 0.0f;

    // Start Game within Menu State
    Breakout.State = GAME_ACTIVE;

    while (!glfwWindowShouldClose(window))
    {
        // Calculate delta time
        GLfloat currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;
        glfwPollEvents();

        //deltaTime = 0.001f;
        // Manage user input
        Breakout.ProcessInput(deltaTime);

        // Update Game state
        Breakout.Update(deltaTime);

        // Render
        glClearColor(0.3f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        glColor3f(1.0f, 0.2f, 0.2f);
        glRectf(10, 190, 10 + 100, 290);

        Breakout.Render();

        //glutSwapBuffers();
        glfwSwapBuffers(window);
    }

    // Delete all resources as loaded using the resource manager
    ResourceManager::Clear();

    glfwTerminate();
    //return 0;

    return a.exec();
}



void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode)
{
    // When a user presses the escape key, we set the WindowShouldClose property to true, closing the application
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GL_TRUE);
    if (key >= 0 && key < 1024)
    {
        if (action == GLFW_PRESS)
            Breakout.Keys[key] = GL_TRUE;
        else if (action == GLFW_RELEASE)
            Breakout.Keys[key] = GL_FALSE;
    }
}

