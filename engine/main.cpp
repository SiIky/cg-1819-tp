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

static int main_window = 0;
static int secondary_window = 0;

static float fov = 45;
static float nearDist = 1;
static float farDist = 1000;
static float Hnear;
static float Wnear;
static float Hfar;
static float Wfar;
static float lX = 0, lY = 0, lZ = 0;
static float uX = 0, uY = 1, uZ = 0;
static struct frustum frst = {0};

void changeSize2 (int w, int h)
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
    gluPerspective(45, ratio, 10, 100000);

    // return to the model view matrix mode
    glMatrixMode(GL_MODELVIEW);
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
    gluPerspective(fov, ratio, nearDist, farDist);
    Hnear = 2 * tan(fov / 2) * nearDist;
    Wnear = Hnear * ratio;
    Hfar = 2 * tan(fov / 2) * farDist;
    Wfar = Hfar * ratio;

    // return to the model view matrix mode
    glMatrixMode(GL_MODELVIEW);
}

float deg2rad (float deg)
{
    return deg * M_PI / 180;
}

static const struct Point L  = Point(0, 0, 0);
static const struct Point Up = Point(0, 1, 0);

static int timebase = 0;
static int frame = 0;
static struct scene scene;

static bool draw_axes   = true;  /* draw axes? */
static bool draw_curves = true;  /* draw Catmull-Rom curves? */
static bool draw_lights = false; /* draw static lights every frame? */

int startX, startY, tracking = 0;
int alpha = 45, beta = 45, r = 50;
float camX = +0, camY = 30, camZ = 40;

void renderScene2 (void)
{
    // clear buffers
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // set the camera
    glLoadIdentity();

    gluLookAt(0, 1000, 0, 0, 0, 0, -1, 0, 0);

    if (draw_axes) {
        glBegin(GL_LINES);
        glVertex3f(0, 0, 0);
        glVertex3f(500, 0, 0);

        glVertex3f(0, 0, 0);
        glVertex3f(0, 500, 0);

        glVertex3f(0, 0, 0);
        glVertex3f(0, 0, 500);
        glEnd();
    }

    unsigned elapsed_program_start = glutGet(GLUT_ELAPSED_TIME);

    sc_draw(&scene, &frst, elapsed_program_start, draw_curves, draw_lights);

    // End of frame
    glutPostRedisplay();
    glutSwapBuffers();
}

void renderScene (void)
{
    // clear buffers
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // set the camera
    glLoadIdentity();

    gluLookAt(camX, camY, camZ, lX, lY, lZ, uX, uY, uZ);

    struct Point p = Point(camX, camY, camZ);
    struct Point l = Point(lX, lY, lZ);
    struct Point u = Point(uX, uY, uZ);
    struct Point d = normalize(l - p);
    struct Point right = normalize(crossProduct(d, u));

    //far points
    struct Point fc = p + (d * farDist);
    struct Point ftl = fc + (u * Hfar / 2) - (right * Wfar / 2);
    struct Point ftr = fc + (u * Hfar / 2) + (right * Wfar / 2);
    struct Point fbl = fc - (u * Hfar / 2) - (right * Wfar / 2);
    struct Point fbr = fc - (u * Hfar / 2) + (right * Wfar / 2);

    //near points
    struct Point nc = p + (d * nearDist);
    struct Point ntl = nc + (u * (Hnear / 2)) - (right * (Wnear / 2));
    struct Point ntr = nc + (u * (Hnear / 2)) + (right * (Wnear / 2));
    struct Point nbl = nc - (u * (Hnear / 2)) - (right * (Wnear / 2));
    struct Point nbr = nc - (u * (Hnear / 2)) + (right * (Wnear / 2));

    frst.far   = Plane(fc,  normalize(crossProduct(ftr - ftl, ftl - fbl)));
    frst.near  = Plane(nc,  normalize(crossProduct(ntl - ntr, ntl - nbl)));
    frst.top   = Plane(ftl, normalize(crossProduct(ntl - ntr, ftr - ntr)));
    frst.bot   = Plane(nbl, normalize(crossProduct(nbr - nbl, fbl - nbl)));
    frst.left  = Plane(ftl, normalize(crossProduct(ntl - ftl, fbl - ftl)));
    frst.right = Plane(ftr, normalize(crossProduct(nbr - fbr, ftr - fbr)));

    if (draw_axes) {
        glBegin(GL_LINES);
        glVertex3f(0, 0, 0);
        glVertex3f(500, 0, 0);

        glVertex3f(0, 0, 0);
        glVertex3f(0, 500, 0);

        glVertex3f(0, 0, 0);
        glVertex3f(0, 0, 500);
        glEnd();
    }

    unsigned elapsed_program_start = glutGet(GLUT_ELAPSED_TIME);
    unsigned elapsed_last_frame = elapsed_program_start - timebase;

    sc_draw(&scene, &frst, elapsed_program_start, draw_curves, draw_lights);

    // End of frame
    glutPostRedisplay();
    glutSwapBuffers();

    frame++;
    if (elapsed_last_frame > 1000) {
        float fps = frame*1000.0/elapsed_last_frame;
        char s[64];
        timebase = elapsed_program_start;
        frame = 0;
        sprintf(s, "FPS: %6.2f", fps);
        glutSetWindowTitle(s);
    }
}

void processKeys (unsigned char c, int xx, int yy)
{
    switch (c) {
        case '#': glPolygonMode(GL_FRONT, GL_FILL);  break;
        case '-': glPolygonMode(GL_FRONT, GL_LINE);  break;
        case '.': glPolygonMode(GL_FRONT, GL_POINT); break;

#define toggle(opt, key) case key: opt = !opt; break
        toggle(draw_axes,   '%');
        toggle(draw_curves, '~');
        toggle(draw_lights, '$');
#undef toggle
    }
}

void processMouseButtons(int button, int state, int xx, int yy)
{
    if (state == GLUT_DOWN) {
        startX = xx;
        startY = yy;
        tracking = (button == GLUT_LEFT_BUTTON) ?
            1:
            (button == GLUT_RIGHT_BUTTON) ?
            2:
            0;
    } else if (state == GLUT_UP) {
        if (tracking == 1) {
            alpha += xx - startX;
            beta += yy - startY;
        } else if (tracking == 2) {
            r -= yy - startY;
            if (r < 3)
                r = 3.0;
        }
        tracking = 0;
    }
}

void processMouseMotion(int xx, int yy)
{
    if (!tracking)
        return;

    int deltaX = xx - startX;
    int deltaY = yy - startY;
    int alphaAux = 0;
    int betaAux = 0;
    int rAux = 0;

    if (tracking == 1) {
        alphaAux = alpha + deltaX;
        betaAux = beta + deltaY;
        if (betaAux > 85.0) { betaAux = 85.0; }
        else if (betaAux < -85.0) { betaAux = -85.0; }
        rAux = r;
    } else if (tracking == 2) {
        alphaAux = alpha;
        betaAux = beta;
        rAux = r - deltaY;
        if (rAux < 3) { rAux = 3; }
    }

    camX = rAux * sin(alphaAux * 3.14 / 180.0) * cos(betaAux * 3.14 / 180.0);
    camZ = rAux * cos(alphaAux * 3.14 / 180.0) * cos(betaAux * 3.14 / 180.0);
    camY = rAux * sin(betaAux * 3.14 / 180.0);
}

int main (int argc, char **argv)
{
    if (argc < 2)
        return usage(*argv);

    // init GLUT and the window
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DEPTH|GLUT_DOUBLE|GLUT_RGBA);

    { /* main window */
        glutInitWindowPosition(100,100);
        glutInitWindowSize(800,800);
        main_window = glutCreateWindow("Main Window");

        // Required callback registry
        glutDisplayFunc(renderScene);
        //glutIdleFunc(renderScene);
        glutReshapeFunc(changeSize);

        // Callback registration for keyboard processing
        glutKeyboardFunc(processKeys);
        glutMouseFunc(processMouseButtons);
        glutMotionFunc(processMouseMotion);

        // OpenGL settings
        glPolygonMode(GL_FRONT, GL_FILL);
        glEnable(GL_DEPTH_TEST);
        glEnable(GL_CULL_FACE);
        glEnableClientState(GL_VERTEX_ARRAY);
        glEnableClientState(GL_NORMAL_ARRAY);
        glEnableClientState(GL_TEXTURE_COORD_ARRAY);
        glEnable(GL_TEXTURE_2D);
        glEnable(GL_LIGHTING);

        glClearColor(0, 0, 0, 0);

#ifndef __APPLE__
        // init GLEW
        glewInit();
#endif

        ilInit();
    }

#if 0
    { /* second window */
        secondary_window = glutCreateWindow("Secondary Window");

        // Required callback registry
        glutDisplayFunc(renderScene2);
        //glutIdleFunc(renderScene2);
        glutReshapeFunc(changeSize2);
        // OpenGL settings
        glPolygonMode(GL_FRONT, GL_FILL);
        glEnable(GL_DEPTH_TEST);
        glEnable(GL_CULL_FACE);
        glEnableClientState(GL_VERTEX_ARRAY);
        glEnableClientState(GL_NORMAL_ARRAY);
        glEnableClientState(GL_TEXTURE_COORD_ARRAY);
        glEnable(GL_TEXTURE_2D);

        glEnable(GL_LIGHTING);
        /* TODO: apagar esta merda */
        glEnable(GL_LIGHT0);
        glEnable(GL_LIGHT1);
        glEnable(GL_LIGHT2);
        glEnable(GL_LIGHT3);
        glEnable(GL_LIGHT4);
        glEnable(GL_LIGHT5);
        glEnable(GL_LIGHT6);
        glEnable(GL_LIGHT7);

        glClearColor(0, 0, 0, 0);

#ifndef __APPLE__
        // init GLEW
        glewInit();
#endif

        ilInit();
    }
#endif /* Secondary Window */


    camX = r * sin(alpha * 3.14 / 180.0) * cos(beta * 3.14 / 180.0);
    camZ = r * cos(alpha * 3.14 / 180.0) * cos(beta * 3.14 / 180.0);
    camY = r * sin(beta * 3.14 / 180.0);

    if (!sc_load_file(argv[1], &scene))
        return !0;

    sc_draw_lights(&scene); /* draw static ligts */

    // enter GLUT's main cycle
    glutMainLoop();

    return !0;
}
