#define _USE_MATH_DEFINES
#include <stdio.h>
#include <stdlib.h>

#ifdef __APPLE__
#include <GLUT/glut.h>
#else
#include <GL/glew.h>
#include <GL/glut.h>
#endif

#include <IL/il.h>

#include "scene.h"
#include <math.h>

#include <vector>
#include <iostream>

#include "../generator/generators.h"

int usage (const char * cmd)
{
    printf("%s SCENE_FILE\n", cmd);
    return !0;
}

void changeSize (int w, int h)
{
    // Prevent a divide by zero, when window is too short
    // (you cant make a window with zero width).
    if(h == 0)
        h = 1;

    // compute window's aspect ratio
    float ratio = w * 1.0 / h;
    // Set the projection matrix as current
    glMatrixMode(GL_PROJECTION);
    // Load Identity Matrix
    glLoadIdentity();

    // Set the viewport to be the entire window
    glViewport(0, 0, w, h);

    // Set perspective
    gluPerspective(45.0f, ratio, 1.0f, 1000.0f);

    // return to the model view matrix mode
    glMatrixMode(GL_MODELVIEW);
}

float deg2rad (float deg)
{
    return deg * M_PI / 180;
}

static const struct Point L  = Point(0, 0, 0);
static const struct Point Up = Point(0, 1, 0);

static float r = 250;
static float a = 45; /* angle in the XZ plane (horizontal) */
static float b = 30; /* angle in the XY plane (vertical) */

static int timebase = 0;
static int frame = 0;
static struct scene scene;

static bool draw_curves = true; /* draw Catmull-Rom curves? */
static bool draw_lights = false; /* draw static lights every frame? */

void renderScene (void)
{
    // clear buffers
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // set the camera
    glLoadIdentity();

    float rc = r * cos(deg2rad(b));

    gluLookAt(
            rc * sin(deg2rad(a)), r * sin(deg2rad(b)), rc * cos(deg2rad(a)),
            L.x, L.y, L.z,
            Up.x, Up.y, Up.z
            );

    glBegin(GL_LINES);
    {
        glColor3ub(255, 0, 0);
        glVertex3f(0, 0, 0);
        glVertex3f(500, 0, 0);

        glColor3ub(0, 255, 0);
        glVertex3f(0, 0, 0);
        glVertex3f(0, 500, 0);

        glColor3ub(0, 0, 255);
        glVertex3f(0, 0, 0);
        glVertex3f(0, 0, 500);
    }
    glEnd();

    unsigned elapsed_program_start = glutGet(GLUT_ELAPSED_TIME);
    unsigned elapsed_last_frame = elapsed_program_start - timebase;

    glColor3ub(100, 100, 100);
    sc_draw(&scene, elapsed_program_start, draw_curves, draw_lights);

    // End of frame
    glutSwapBuffers();

    frame++;
    if (elapsed_last_frame > 1000) {
        float fps = frame*1000.0/elapsed_last_frame;
        char s[64];
        timebase = elapsed_program_start;
        frame = 0;
        sprintf(s, "FPS: %f6.2", fps);
        glutSetWindowTitle(s);
    }
}

void processKeys (unsigned char c, int xx, int yy)
{
#define toggle(opt, key) case key: opt = !opt; break
    switch (c) {
        case 'w': b += 1; break;
        case 'a': a -= 1; break;
        case 's': b -= 1; break;
        case 'd': a += 1; break;

        case 'k': r -= 1; break;
        case 'j': r += 1; break;

        case '#': glPolygonMode(GL_FRONT, GL_FILL);  break;
        case '-': glPolygonMode(GL_FRONT, GL_LINE);  break;
        case '.': glPolygonMode(GL_FRONT, GL_POINT); break;

        toggle(draw_curves, '~');
        toggle(draw_lights, '$');
    }
}

int main (int argc, char **argv)
{
    if (argc < 2)
        return usage(*argv);

    // init GLUT and the window
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DEPTH|GLUT_DOUBLE|GLUT_RGBA);
    glutInitWindowPosition(100,100);
    glutInitWindowSize(800,800);
    glutCreateWindow("CG@DI-UM");
    glPolygonMode(GL_FRONT, GL_FILL);

    // Required callback registry
    glutDisplayFunc(renderScene);
    glutIdleFunc(renderScene);
    glutReshapeFunc(changeSize);

    // Callback registration for keyboard processing
    glutKeyboardFunc(processKeys);

    // OpenGL settings
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_NORMAL_ARRAY);

    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    glEnable(GL_LIGHT1);
    glEnable(GL_LIGHT2);
    glEnable(GL_LIGHT3);
    glEnable(GL_LIGHT4);
    glEnable(GL_LIGHT5);
    glEnable(GL_LIGHT6);
    glEnable(GL_LIGHT7);

#ifndef __APPLE__
    // init GLEW
    glewInit();
#endif

    ilInit();

    if (!sc_load_file(argv[1], &scene))
        return !0;

    sc_draw_lights(&scene); /* draw static ligts */

    // enter GLUT's main cycle
    glutMainLoop();

    return !0;
}
