#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <iostream>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <opencv2/opencv.hpp>
#include <opencv2/core/core.hpp>
#include "opencv2/core/utility.hpp"


#define new_max(x,y) ((x) >= (y)) ? (x) : (y)
#define new_min(x,y) ((x) <= (y)) ? (x) : (y)

using std::cout;
using std::endl;

int window_width  = 640;
int window_height = 480;

// Frame counting and limiting
int    frame_count = 0;
double frame_start_time, frame_end_time, frame_draw_time;

// Function turn a cv::Mat into a texture, and return the texture ID as a GLuint for use
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

static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, GLFW_TRUE);
    }
}

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




cv::Ptr<cv::BackgroundSubtractorMOG2> pMOG2; //MOG2 Background subtractor

cv::RNG rng( 0xFFFFFFFF );
int main(int argc, char **argv)
{
   // if (argc != 2) {
   //     cout << "Usage: " << argv[0] << "<path_to_video_file>" << endl;
   //     exit(EXIT_FAILURE);
   // }

    //cv::VideoCapture capture(argv[1]);
    cv::VideoCapture capture(0);
    //cv::VideoCapture capture("/home/kostasl/nextcloud/tomFlu.mp4");
    if (!capture.isOpened()) {
        cout << "Cannot open video: " << argv[1] << endl;
        exit(EXIT_FAILURE);
    }

    double fps = 0.0;
    fps = capture.get(CV_CAP_PROP_FPS);
    if (fps != fps) { // NaN
        fps = 25.0;
    }

    cout << "FPS: " << fps << endl;

    window_width = capture.get(CV_CAP_PROP_FRAME_WIDTH);
    window_height = capture.get(CV_CAP_PROP_FRAME_HEIGHT);
    cout << "Video width: " << window_width << endl;
    cout << "Video height: " << window_height << endl;

    GLFWwindow* window;

    glfwSetErrorCallback(error_callback);

    if (!glfwInit()) {
        exit(EXIT_FAILURE);
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
    window = glfwCreateWindow(window_width, window_height, "On Fire Lab", NULL, NULL);
    if (!window) {
        glfwTerminate();
        exit(EXIT_FAILURE);
    }

    glfwSetKeyCallback(window, key_callback);
    glfwSetWindowSizeCallback(window, resize_callback);

    glfwMakeContextCurrent(window);

    glfwSwapInterval(1);

    //  Initialise glew (must occur AFTER window creation or glew will error)
    GLenum err = glewInit();
    if (GLEW_OK != err)
    {
        cout << "GLEW initialisation error: " << glewGetErrorString(err) << endl;
        exit(-1);
    }
    cout << "GLEW okay - using version: " << glewGetString(GLEW_VERSION) << endl;

    init_opengl(window_width, window_height);

    double video_start_time = glfwGetTime();
    double video_end_time = 0.0;


    //Doesn't matter if cuda FLAG is enabled
    int MOGhistory = 30;
    float gdMOGBGRatio = 0.9f;
    pMOG2 =  cv::createBackgroundSubtractorMOG2(MOGhistory, 20,false);

    pMOG2->setHistory(MOGhistory);
    pMOG2->setNMixtures(20);
    pMOG2->setBackgroundRatio(gdMOGBGRatio); ///

    cv::Mat frame_live,frame_live_prev,frame_MOG,frame_out;
    cv::Mat frame_live_grey,frame_live_prev_grey,frame_OptFlow,frame_out_HSV;
    cv::Mat frame_GFP_mask,frame_GFP_mask_col,frame_GFP_mask_col_acc;

    std::vector<cv::KeyPoint> vKpt_next;
    std::vector<cv::Point2f> vpt_next,vpt_current,vpt_motion;
    std::vector<uchar> voutStatus;
    // L1 distance between patches around the original and a moved point, divided by number of pixels in a window, is used as a error measure.
    std::vector<float>    voutError;

    vpt_current.push_back(cv::Point2f(400,10));

    while (!glfwWindowShouldClose(window)) {
        frame_start_time = glfwGetTime();
        if (!capture.read(frame_live)) {
            cout << "Cannot grab a frame." << endl;
            break;
        }



        pMOG2->apply(frame_live,frame_MOG,0.05);
        video_end_time = glfwGetTime();

        //if (frame_MOG.depth() != CV_8U)
        //    cv::cvtColor( frame_MOG, frame_out, cv::COLOR_GRAY2BGR );
        //else
        //    frame_MOG.copyTo(frame_out);

        cv::cvtColor( frame_live, frame_out_HSV, cv::COLOR_BGR2HSV );
        cv::inRange(frame_out_HSV,cv::Scalar(50,0,170,0),cv::Scalar(150,255,255),frame_GFP_mask);


        cv::bitwise_and(frame_MOG,frame_GFP_mask,frame_GFP_mask);
        cv::cvtColor( frame_GFP_mask, frame_GFP_mask_col, cv::COLOR_GRAY2BGR );

        if (frame_GFP_mask_col_acc.empty())
            frame_GFP_mask_col_acc = cv::Mat::zeros(frame_GFP_mask_col.rows,frame_GFP_mask_col.cols,frame_GFP_mask_col.type() );


        float alpha = 0.8;
        float beta = ( 1.0 - alpha );



        frame_GFP_mask_col.setTo(cv::Scalar(0,0,250,100),frame_GFP_mask);
        //cv::addWeighted(frame_GFP_mask_col,alpha,frame_GFP_mask_col,beta,0.0,frame_GFP_mask_col);
        frame_GFP_mask_col_acc = frame_GFP_mask_col_acc*0.97+ frame_GFP_mask_col;
       // cv::accumulate(frame_GFP_mask_col,frame_GFP_mask_col_acc);

        //cv::accumulate(frame_GFP_mask_col,frame_GFP_mask_col);
        //frame_GFP_mask_col = frame_GFP_mask_col*0.90;



        //frame_out.copyTo(frame_out,frame_GFP_mask)
        //frame_GFP_mask.copyTo();
        alpha = 0.5;
        cv::addWeighted( frame_GFP_mask_col_acc, alpha, frame_live, beta, 0.0, frame_out);

        //frame_live.copyTo(frame_out,frame_MOG);
        //frame_live.copyTo(frame_out);

        //Calc Optic Flow for each food item
        if (frame_count > MOGhistory)
        {
            //cv::calcOpticalFlowPyrLK(frame_live_prev,frame_out,vpt_current,vpt_next,voutStatus,voutError,cv::Size(51,51),3);
//            for (int i=0;i<(int)vpt_next.size() && i< 100;i++)
//            {
//                if (!voutStatus.at(i))
//                    continue; //ignore bad point
//                vpt_motion.push_back(vpt_next.at(i));
//            }
//            cv::KeyPoint::convert(vpt_motion,vKpt_next);
//            cv::drawKeypoints(frame_out,vKpt_next,frame_out,CV_RGB(255,30,30),cv::DrawMatchesFlags::DRAW_RICH_KEYPOINTS);

            /////Diffusive Optic Flow -- Big Lag
//            cv::cvtColor( frame_live, frame_live_grey, cv::COLOR_BGR2GRAY );
//            cv::cvtColor( frame_live_prev, frame_live_prev_grey, cv::COLOR_BGR2GRAY );
//            //img2.copyTo(img2OriginalC);
//             cv::calcOpticalFlowFarneback(frame_live_grey, frame_live_prev_grey, frame_OptFlow, .5, 1, 50, 1, 3, 1.2, 0);

//             for (int y = 0; y < frame_live_prev_grey.rows; y += 50) {
//                         for (int x = 0; x < frame_live_prev_grey.cols; x += 50)
//                         {
//                             // get the flow from y, x position * 3 for better visibility
//                             const cv::Point2f flowatxy = frame_OptFlow.at<cv::Point2f>(y, x) *10;
//                             // draw line at flow direction
//                             if (cv::norm(flowatxy) > 10 ){
//                                 int colRedShift = rng.uniform(100, new_min(100,new_min((int)cv::norm(flowatxy), 200)) );
//                                 int colRadShift = rng.uniform(3, new_min((int)cv::norm(flowatxy)/2, 10) );

//                                    cv::arrowedLine(frame_out, cv::Point(x, y),
//                                                    cv::Point(cvRound(x + flowatxy.x), cvRound(y + flowatxy.y)),
//                                                    cv::Scalar(0, 50, 55+colRedShift,50),4);
//                                    // draw initial point

//                                    cv::circle(frame_out, cv::Point(x, y), colRadShift, cv::Scalar(0, 30, 55+colRedShift,30), -1);
//                                }
//                         }
//               }


            //Update the optic flow key points
            vpt_current =  vpt_motion;
         }
         draw_frame(frame_out);

        glfwSwapBuffers(window);
        glfwPollEvents();

        ++frame_count;

        frame_out.copyTo(frame_live_prev);
       // lock_frame_rate(fps);
    }

    cout << "Total video time: " << video_end_time - video_start_time << " seconds" << endl;

    capture.release();
    glfwDestroyWindow(window);
    glfwTerminate();

    exit(EXIT_SUCCESS);
}
