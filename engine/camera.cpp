#ifdef __APPLE__
#include <GLUT/glut.h>
#else
#include <GL/glew.h>
#include <GL/glut.h>
#endif
#define _USE_MATH_DEFINES
#include "camera.h"
#include <math.h>

#include <assert.h>

#include "../generator/generators.h"

#include <stdio.h>

static float deg2rad (float deg)
{ return deg * M_PI / 180; }

static float rad2deg (float rad)
{ return rad / M_PI * 180; }

static struct Point rab2P (float r, float a, float b)
{
    return r * Point(
            sin(deg2rad(a)) * cos(deg2rad(b)),
            sin(deg2rad(b)),
            cos(deg2rad(a)) * cos(deg2rad(b))
            );
}

static inline void cam_process_keys_fps (struct cam * cam, unsigned char c, int x, int y, struct Point * P, struct Point * L)
{
    struct Point R = normalize(crossProduct(cam->atr.fps.D, Point(0, 1, 0)));
    switch (c) {
        case 'w':
            cam->atr.fps.P = cam->atr.fps.P + cam->atr.fps.D;
            break;
        case 'a':
            cam->atr.fps.P = cam->atr.fps.P - R;
            break;
        case 's':
            cam->atr.fps.P = cam->atr.fps.P - cam->atr.fps.D;
            break;
        case 'd':
            cam->atr.fps.P = cam->atr.fps.P + R;
            break;
    }
    *P = cam->atr.fps.P;
    *L = cam->atr.fps.P + cam->atr.fps.D;
}

static inline void cam_process_keys_exp (struct cam * cam, unsigned char c, int x, int y, struct Point * P, struct Point * L)
{
    switch (c) {
        case 'w': cam->atr.exp.b += 1; break;
        case 'a': cam->atr.exp.a -= 1; break;
        case 's': cam->atr.exp.b -= 1; break;
        case 'd': cam->atr.exp.a += 1; break;

        case 'k': cam->atr.exp.r -= 1; break;
        case 'j': cam->atr.exp.r += 1; break;
    }
    *P = rab2P(cam->atr.exp.r, cam->atr.exp.a, cam->atr.exp.b);
    *L = Point(0, 0, 0);

    fprintf(stderr, "%f %f %f\n", cam->atr.exp.r, cam->atr.exp.a, cam->atr.exp.b);
}

void cam_process_keys (struct cam * cam, unsigned char c, int x, int y, struct Point * P, struct Point * L)
{
    switch (cam->type) {
        case CAM_EXP: cam_process_keys_exp(cam, c, x, y, P, L);
        case CAM_FPS: cam_process_keys_fps(cam, c, x, y, P, L);
    }
}

void cam_init_exp (struct cam * cam, float r, float a, float b, struct Point * P)
{
    cam->type = CAM_EXP;
    cam->atr.exp.r = r;
    cam->atr.exp.a = a;
    cam->atr.exp.b = b;
    *P = rab2P(r, a, b);
}

void cam_init_fps (struct cam * cam, struct Point P, struct Point D, struct Point * Pout, struct Point * Lout)
{
    cam->type = CAM_FPS;
    cam->atr.fps.P = P;
    cam->atr.fps.D = D;
    *Pout = P;
    *Lout = P + D;
}

static void cam_switch2fps (struct cam * cam, struct Point * P, struct Point * L)
{
    cam->type = CAM_FPS;
    *L = Point(0, 0, 0);
    cam->atr.fps.P = *P = rab2P(cam->atr.exp.r, cam->atr.exp.a, cam->atr.exp.b);
    cam->atr.fps.D = Point(0, 0, 0) - normalize(cam->atr.fps.P);
}

static void cam_switch2exp (struct cam * cam, struct Point * P, struct Point * L)
{
    cam->type = CAM_EXP;
    float x = cam->atr.fps.P.x;
    float y = cam->atr.fps.P.y;
    float z = cam->atr.fps.P.z;

    *L = Point(0, 0, 0);
    *P = cam->atr.fps.P;

    float a = cam->atr.exp.a = rad2deg(atan(y / sqrt(x*x + z*z)));
    float b = cam->atr.exp.b = rad2deg(atan(x / z));
    float r = cam->atr.exp.r = norm(Point(x, y, z));

    fprintf(stderr, "%f, %f, %f\n", r, a, b);
}

void cam_switch_mode (struct cam * cam, struct Point * P, struct Point * L)
{
    if (cam->type == CAM_EXP) {
        cam_switch2fps(cam, P, L);
    } else if (cam->type == CAM_FPS) {
        cam_switch2exp(cam, P, L);
    } else assert(!"unreachable");
}
