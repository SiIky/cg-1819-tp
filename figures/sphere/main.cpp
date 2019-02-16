#ifdef __APPLE__
#include <GLUT/glut.h>
#else
#include <GL/glut.h>
#endif

#include <math.h>

#include <vector>
#include <tuple>

void drawSphere (unsigned r, unsigned stacks, unsigned slices)
{
    std::vector<std::tuple<float, float, float>> verts;

    for (unsigned i = 0; i <= stacks; i++) {
        float p = ((float) i) / ((float) stacks) * M_PI;

        for (unsigned j = 0; j <= slices; j++) {
            float t = ((float) j) / ((float) slices) * 2 * M_PI;

            float ct = cosf(t);
            float sp = sinf(p);
            float cp = cosf(p);
            float st = sinf(t);

            std::tuple<float, float, float> tmp;
            std::get<0>(tmp) = r * ct * sp;
            std::get<1>(tmp) = r * cp;
            std::get<2>(tmp) = r * st * sp;

            verts.push_back(tmp);
        }
    }

    glBegin(GL_TRIANGLES); {
        glColor3ub(200, 200, 200);

        unsigned len = slices * stacks + slices;
        for (unsigned i = 0; i < len; i++) {
            auto P1 = verts.at(i);
            auto P2 = verts.at(i + slices + 1);
            auto P3 = verts.at(i + slices);
            auto P4 = verts.at(i + 1);

            glVertex3f(std::get<0>(P1), std::get<1>(P1), std::get<2>(P1));
            glVertex3f(std::get<0>(P2), std::get<1>(P2), std::get<2>(P2));
            glVertex3f(std::get<0>(P3), std::get<1>(P3), std::get<2>(P3));

            glVertex3f(std::get<0>(P2), std::get<1>(P2), std::get<2>(P2));
            glVertex3f(std::get<0>(P1), std::get<1>(P1), std::get<2>(P1));
            glVertex3f(std::get<0>(P4), std::get<1>(P4), std::get<2>(P4));
        }
    } glEnd();
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


void renderScene (void)
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();
    gluLookAt(
            10, 10, 10, 
            0, 0, 0,
            0, 1, 0
            );

    glBegin(GL_LINES); {
        glColor3ub(255, 0, 0);
        glVertex3f(0, 0, 0);
        glVertex3f(10, 0, 0);

        glColor3ub(0, 255, 0);
        glVertex3f(0, 0, 0);
        glVertex3f(0, 10, 0);

        glColor3ub(0, 0, 255);
        glVertex3f(0, 0, 0);
        glVertex3f(0, 0, 10);
    } glEnd();

    drawSphere(4, 20, 20);

    glutSwapBuffers();
}

int main (int argc, char **argv)
{
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DEPTH|GLUT_DOUBLE|GLUT_RGBA);
    glutInitWindowPosition(100,100);
    glutInitWindowSize(800,800);
    glutCreateWindow("CG@DI-UM");

    glutDisplayFunc(renderScene);
    glutReshapeFunc(changeSize);

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);

    glutMainLoop();

    return 1;
}
