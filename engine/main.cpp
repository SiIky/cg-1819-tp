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
#include "camera.h"

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
static struct Point L = Point(0, 0, 0);
static struct Point U = Point(0, 1, 0);
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
    glutPostRedisplay();
}

static int timebase = 0;
static int frame = 0;
static struct scene scene;

static bool draw_axes   = true;  /* draw axes? */
static bool draw_curves = true;  /* draw Catmull-Rom curves? */
static bool draw_lights = false; /* draw static lights every frame? */

int startX, startY, tracking = 0;
int alpha = 45, beta = 45, r = 10;

void renderScene2 (void)
{
    // clear buffers
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // set the camera
    glLoadIdentity();

    gluLookAt(0, 1000, 0, 0, 0, 0, -1, 0, 0);

    if (draw_axes) {
        glBegin(GL_LINES);
        glColor3ub(255, 0, 0);
        glVertex3f(0, 0, 0);
        glVertex3f(500, 0, 0);

        glColor3ub(0, 255, 0);
        glVertex3f(0, 0, 0);
        glVertex3f(0, 500, 0);

        glColor3ub(0, 0, 255);
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

static struct Point P = Point(0, 0, 0);

void renderScene (void)
{
    // clear buffers
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // set the camera
    glLoadIdentity();

    gluLookAt(
            P.x, P.y, P.z,
            L.x, L.y, L.z,
            U.x, U.y, U.z
            );

    struct Point d = normalize(L - P);
    struct Point right = normalize(crossProduct(d, U));

    //far points
    struct Point fc = P + (d * farDist);
    struct Point ftl = fc + (U * Hfar / 2) - (right * Wfar / 2);
    struct Point ftr = fc + (U * Hfar / 2) + (right * Wfar / 2);
    struct Point fbl = fc - (U * Hfar / 2) - (right * Wfar / 2);
    struct Point fbr = fc - (U * Hfar / 2) + (right * Wfar / 2);

    //near points
    struct Point nc = P + (d * nearDist);
    struct Point ntl = nc + (U * (Hnear / 2)) - (right * (Wnear / 2));
    struct Point ntr = nc + (U * (Hnear / 2)) + (right * (Wnear / 2));
    struct Point nbl = nc - (U * (Hnear / 2)) - (right * (Wnear / 2));
    struct Point nbr = nc - (U * (Hnear / 2)) + (right * (Wnear / 2));

    frst.far   = Plane(fc,  normalize(crossProduct(ftr - ftl, ftl - fbl)));
    frst.near  = Plane(nc,  normalize(crossProduct(ntl - ntr, ntl - nbl)));
    frst.top   = Plane(ftl, normalize(crossProduct(ntl - ntr, ftr - ntr)));
    frst.bot   = Plane(nbl, normalize(crossProduct(nbr - nbl, fbl - nbl)));
    frst.left  = Plane(ftl, normalize(crossProduct(ntl - ftl, fbl - ftl)));
    frst.right = Plane(ftr, normalize(crossProduct(nbr - fbr, ftr - fbr)));

    if (draw_axes) {
        glBegin(GL_LINES);
        glColor3ub(255, 0, 0);
        glVertex3f(0, 0, 0);
        glVertex3f(500, 0, 0);

        glColor3ub(0, 255, 0);
        glVertex3f(0, 0, 0);
        glVertex3f(0, 500, 0);

        glColor3ub(0, 0, 255);
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

static struct cam cam;

void processKeys (unsigned char c, int xx, int yy)
{
    switch (c) {
        case '#': glPolygonMode(GL_FRONT, GL_FILL);  break;
        case '-': glPolygonMode(GL_FRONT, GL_LINE);  break;
        case '.': glPolygonMode(GL_FRONT, GL_POINT); break;
        case 'c': cam_switch_mode(&cam, &P, &L);

#define toggle(opt, key) case key: opt = !opt; break
                  toggle(draw_axes,   '%');
                  toggle(draw_curves, '~');
                  toggle(draw_lights, '$');
#undef toggle

        default: cam_process_keys(&cam, c, xx, yy, &P, &L);
    }
    glutPostRedisplay();
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

    P.x = rAux * sin(alphaAux * 3.14 / 180.0) * cos(betaAux * 3.14 / 180.0);
    P.z = rAux * cos(alphaAux * 3.14 / 180.0) * cos(betaAux * 3.14 / 180.0);
    P.y = rAux * sin(betaAux * 3.14 / 180.0);
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


    //P.x = r * sin(alpha * 3.14 / 180.0) * cos(beta * 3.14 / 180.0);
    //P.y = r * sin(beta * 3.14 / 180.0);
    //P.z = r * cos(alpha * 3.14 / 180.0) * cos(beta * 3.14 / 180.0);

    if (!sc_load_file(argv[1], &scene))
        return !0;

    sc_draw_lights(&scene); /* draw static ligts */
    //cam_init_fps(&cam, Point(1, 1, 1), Point(-1, -1, -1), &P, &L);
    cam_init_exp(&cam, r, alpha, beta, &P);

    fprintf(stderr, "%f %f %f\n", cam.atr.exp.r, cam.atr.exp.a, cam.atr.exp.b);

    // enter GLUT's main cycle
    glutMainLoop();

    return !0;
}
