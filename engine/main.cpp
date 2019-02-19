#ifdef __APPLE__
#include <GLUT/glut.h>
#else
#include <GL/glut.h>
#endif

#define _USE_MATH_DEFINES
#include <math.h>
#include <stdio.h>

#include <vector>

#include "../generator/generators.h"

int usage (const char * cmd)
{
    printf("%s 3DFILE\n", cmd);
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
    gluPerspective(45.0f ,ratio, 1.0f ,1000.0f);

    // return to the model view matrix mode
    glMatrixMode(GL_MODELVIEW);
}

float deg2rad (float deg)
{
    return deg * M_PI / 180;
}

static const struct Point L  = Point(0, 0, 0);
static const struct Point Up = Point(0, 1, 0);

static const float r = 10;
static float a = 315;
static float b = 45;

static std::vector<struct Point> TrianglePoints;

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

    glBegin(GL_TRIANGLES); {
        glColor3ub(100, 100, 100);
        for (auto t : TrianglePoints)
            glVertex3f(t.x, t.y, t.z);
    } glEnd();

    // End of frame
    glutSwapBuffers();
}

void processKeys(unsigned char c, int xx, int yy)
{
    switch (c) {
        case 'w': {
            b += 1;
        } break;
        case 's': {
            b -= 1;
        } break;
        case 'a': {
            a -= 1;
        } break;
        case 'd': {
            a += 1;
        } break;
    }

    glutPostRedisplay();
}

int main (int argc, char **argv)
{
    if (argc < 2)
        return usage(*argv);

    {
        FILE * inf = fopen(argv[1], "r");
	gen_model_read(inf, &TrianglePoints);
	fclose(inf);
    }

    // init GLUT and the window
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DEPTH|GLUT_DOUBLE|GLUT_RGBA);
    glutInitWindowPosition(100,100);
    glutInitWindowSize(800,800);
    glutCreateWindow("CG@DI-UM");

    // Required callback registry 
    glutDisplayFunc(renderScene);
    glutReshapeFunc(changeSize);

    // Callback registration for keyboard processing
    glutKeyboardFunc(processKeys);

    // OpenGL settings
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);

    // enter GLUT's main cycle
    glutMainLoop();

    return 1;
}
