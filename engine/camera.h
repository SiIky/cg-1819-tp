#ifndef _CAMERA_H
#define _CAMERA_H

#include "../generator/generators.h"

enum cam_type {
    CAM_EXP,
    CAM_FPS,
};

struct cam {
    enum cam_type type;

    union {
        struct {
            float r;
            float a;
            float b;
        } exp;

        struct {
            struct Point P;
            struct Point D;
        } fps;
    } atr;
};

void cam_init_exp     (struct cam * cam, float r, float a, float b, struct Point * P);
void cam_init_fps     (struct cam * cam, struct Point P, struct Point D, struct Point * Pout, struct Point * Lout);
void cam_process_keys (struct cam * cam, unsigned char c, int x, int y, struct Point * P, struct Point * L);
void cam_switch_mode  (struct cam * cam, struct Point * P, struct Point * L);

#endif /* _CAMERA_H */
