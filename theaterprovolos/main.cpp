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

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>


#define GLEW_STATIC
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "game.h"
#include "resource_manager.h"

///OpenCV

#include <opencv2/opencv.hpp>
#include <opencv2/core/core.hpp>
#include "opencv2/core/utility.hpp"


// GLFW function declerations
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode);

// The Width of the screen
const GLuint SCREEN_WIDTH = 1280;
// The height of the screen
const GLuint SCREEN_HEIGHT = 768;

Game Breakout (SCREEN_WIDTH, SCREEN_HEIGHT);


using std::cout;
using std::endl;

int window_width  = SCREEN_WIDTH;
int window_height = SCREEN_HEIGHT;

// Frame counting and limiting
int    frame_count = 0;
double frame_start_time, frame_end_time, frame_draw_time;

///// For Drawing Frames On OPENGL //
/// Function turn a cv::Mat into a texture, and return the texture ID as a GLuint for use
static GLuint matToTexture(const cv::Mat &mat, GLenum minFilter, GLenum magFilter, GLenum wrapFilter) {
    // Generate a number for our textureID's unique handle
    GLuint textureID;
    glGenTextures(1, &textureID);

    // Bind to our texture handle
    glBindTexture(GL_TEXTURE_2D, textureID);

    // Catch silly-mistake texture interpolation method for magnification
    if (magFilter == GL_LINEAR_MIPMAP_LINEAR  ||
            magFilter == GL_LINEAR_MIPMAP_NEAREST ||
            magFilter == GL_NEAREST_MIPMAP_LINEAR ||
            magFilter == GL_NEAREST_MIPMAP_NEAREST)
    {
        cout << "You can't use MIPMAPs for magnification - setting filter to GL_LINEAR" << endl;
        magFilter = GL_LINEAR;
    }

    // Set texture interpolation methods for minification and magnification
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, minFilter);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, magFilter);

    // Set texture clamping method
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrapFilter);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrapFilter);

    // Set incoming texture format to:
    // GL_BGR       for CV_CAP_OPENNI_BGR_IMAGE,
    // GL_LUMINANCE for CV_CAP_OPENNI_DISPARITY_MAP,
    // Work out other mappings as required ( there's a list in comments in main() )
    GLenum inputColourFormat = GL_BGR;
    if (mat.channels() == 1)
    {
        inputColourFormat = GL_LUMINANCE;
    }

    // Create the texture
    glTexImage2D(GL_TEXTURE_2D,     // Type of texture
                 0,                 // Pyramid level (for mip-mapping) - 0 is the top level
                 GL_RGB,            // Internal colour format to convert to
                 mat.cols,          // Image width  i.e. 640 for Kinect in standard mode
                 mat.rows,          // Image height i.e. 480 for Kinect in standard mode
                 0,                 // Border width in pixels (can either be 1 or 0)
                 inputColourFormat, // Input image format (i.e. GL_RGB, GL_RGBA, GL_BGR etc.)
                 GL_UNSIGNED_BYTE,  // Image data type
                 mat.ptr());        // The actual image data itself

    // If we're using mipmaps then generate them. Note: This requires OpenGL 3.0 or higher
    if (minFilter == GL_LINEAR_MIPMAP_LINEAR  ||
            minFilter == GL_LINEAR_MIPMAP_NEAREST ||
            minFilter == GL_NEAREST_MIPMAP_LINEAR ||
            minFilter == GL_NEAREST_MIPMAP_NEAREST)
    {
        glGenerateMipmap(GL_TEXTURE_2D);
    }

    return textureID;
}

static void error_callback(int error, const char* description) {
    fprintf(stderr, "Error: %s\n", description);
}

//static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
//    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
//        glfwSetWindowShouldClose(window, GLFW_TRUE);
//    }
//}

static void resize_callback(GLFWwindow* window, int new_width, int new_height) {
    glViewport(0, 0, window_width = new_width, window_height = new_height);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0.0, window_width, window_height, 0.0, 0.0, 100.0);
    glMatrixMode(GL_MODELVIEW);
}

static void draw_frame(const cv::Mat& frame) {
    // Clear color and depth buffers
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glMatrixMode(GL_MODELVIEW);     // Operate on model-view matrix

    glEnable(GL_TEXTURE_2D);
    GLuint image_tex = matToTexture(frame, GL_LINEAR_MIPMAP_LINEAR, GL_LINEAR, GL_CLAMP);

    /* Draw a quad */
    glBegin(GL_QUADS);
    glTexCoord2i(0, 0); glVertex2i(0,   0);
    glTexCoord2i(0, 1); glVertex2i(0,   window_height);
    glTexCoord2i(1, 1); glVertex2i(window_width, window_height);
    glTexCoord2i(1, 0); glVertex2i(window_width, 0);
    glEnd();

    glDeleteTextures(1, &image_tex);
    glDisable(GL_TEXTURE_2D);
}

void lock_frame_rate(double frame_rate) {
    static double allowed_frame_time = 1.0 / frame_rate;

    // Note: frame_start_time is called first thing in the main loop
    frame_end_time = glfwGetTime();  // in seconds

    frame_draw_time = frame_end_time - frame_start_time;

    double sleep_time = 0.0;

    if (frame_draw_time < allowed_frame_time) {
        sleep_time = allowed_frame_time - frame_draw_time;
        usleep(1000000 * sleep_time);
    }

    // Debug stuff
    double potential_fps = 1.0 / frame_draw_time;
    double locked_fps    = 1.0 / (glfwGetTime() - frame_start_time);
    cout << "Frame [" << frame_count << "] ";
    cout << "Draw: " << frame_draw_time << " Sleep: " << sleep_time;
    cout << " Pot. FPS: " << potential_fps << " Locked FPS: " << locked_fps << endl;
}

static void init_opengl(int w, int h) {
    glViewport(0, 0, w, h); // use a screen size of WIDTH x HEIGHT

    glMatrixMode(GL_PROJECTION);     // Make a simple 2D projection on the entire window
    glLoadIdentity();
    glOrtho(0.0, w, h, 0.0, 0.0, 100.0);

    glMatrixMode(GL_MODELVIEW);    // Set the matrix mode to object modeling

    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClearDepth(0.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // Clear the window
}

/////



using namespace cv;
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

    glEnable(GL_CULL_FACE);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Initialize game
    Breakout.Init();

    // DeltaTime variables
    GLfloat deltaTime = 0.0f;
    GLfloat lastFrame = 0.0f;

    // Start Game within Menu State
    Breakout.State = GAME_ACTIVE;

////
    VideoCapture cap(0); // open the default camera
       if(!cap.isOpened())  // check if we succeeded
           return -1;

       Mat edges;
      // cv::namedWindow("edges",CV_WINDOW_NORMAL);
       //cv::waitKey(1);

    init_opengl(window_width, window_height);

    while (!glfwWindowShouldClose(window))
    {

        Mat frame;
        cap.read(frame); // get a new frame from camera
        cv::write("\home\kostasl\xxx.png",)
        //cvtColor(frame, edges, COLOR_BGR2GRAY);
        //GaussianBlur(edges, edges, Size(7,7), 1.5, 1.5);
        //Canny(edges, edges, 0, 30, 3);
        //imshow("edges", frame);
        //cv::waitKey(1);
        //video_end_time = glfwGetTime();


        // Calculate delta time
        GLfloat currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        //deltaTime = 0.001f;
        // Manage user input
       // Breakout.ProcessInput(deltaTime);

        // Update Game state
        //Breakout.Update(deltaTime);

        // Render
        //glClearColor(0.3f, 0.3f, 0.3f, 1.0f);
        //glClear(GL_COLOR_BUFFER_BIT);

        draw_frame(frame);

        //glColor3f(1.0f, 0.2f, 0.2f);
        //glRectf(10, 190, 10 + 100, 290);

        Breakout.Render();

        //glutSwapBuffers();
        glfwSwapBuffers(window);
        glfwPollEvents();
        //lock_frame_rate(fps);

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

