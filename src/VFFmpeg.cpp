// VFFmpeg.cpp : Defines the entry point for the application.
//

#include "VFFmpeg.h"
#include <GLFW/glfw3.h>
#include <stdio.h>

bool load_frame(const char* filename, int* frame_width, int* frame_height, unsigned char** data);
int main(int argc, char** argv) {

    GLFWwindow* window;

    /* Initialize the library */
    if (!glfwInit())
        return -1;

    /* Create a windowed mode window and its OpenGL context */
    window = glfwCreateWindow(1280, 720, "Hello World", NULL, NULL);
    if (!window)
    {
        glfwTerminate();
        return -1;
    }
    

    /* Make the window's context current */
    int frame_width, frame_height;
    unsigned char* data;
    if (!load_frame("C:\\Users\\hp\\Down\loads\\Video\\LIVESTREAM- Setting up FFmpeg and OpenGL in C++ for real-time video processing.mp4", &frame_width, &frame_height, &data)) {
        printf("couldn't laod frame\n");
        return -1;
    }
    glfwMakeContextCurrent(window);

    GLuint tex_handle;
    glGenTextures(1, &tex_handle);
    glBindTexture(GL_TEXTURE_2D, tex_handle);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, frame_width, frame_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);

    /* Loop until the user closes the window */
    while (!glfwWindowShouldClose(window))
    {
        /* Render here */
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        int window_frame_width, window_frame_height;
        glfwGetFramebufferSize(window, &window_frame_width, &window_frame_height);
        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        glOrtho(0, window_frame_width, window_frame_height, 0, -1, 1);
        glMatrixMode(GL_MODELVIEW);
        glEnable(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, tex_handle);
        glBegin(GL_QUADS);
        glTexCoord2d(0, 0); glVertex2i(200, 200);
        glTexCoord2d(1, 0); glVertex2i(200 + frame_width, 200);
        glTexCoord2d(1, 1); glVertex2i(200 + frame_width, 200 + frame_height);
        glTexCoord2d(0, 1); glVertex2i(200, 200 + frame_height);

        glEnd();
        glDisable(GL_TEXTURE_2D);
        /* Swap front and back buffers */
        glfwSwapBuffers(window);

        /* Poll for and process events */
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}
