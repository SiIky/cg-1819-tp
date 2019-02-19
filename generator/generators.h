#ifndef _GENERATORS_H
#define _GENERATORS_H

#include <stdbool.h>
#include <stdio.h>

struct Point {
    float x;
    float y;
    float z;
};

struct Triangle {
    struct Point P1;
    struct Point P2;
    struct Point P3;
};

struct Rectangle {
    struct Point P1;
    struct Point P2;
    struct Point P3;
    struct Point P4;
};

struct Box {
    struct Rectangle top;
    struct Rectangle bottom;
};

struct Cone {
    float rad;
    float height;
    unsigned ndivs;
};

struct Cylinder {
    float rad;
    float height;
    unsigned ndivs;
};

struct Sphere {
    float rad;
    unsigned slices;
    unsigned stacks;
};

/**
 * P1---P3
 * |    /
 * |   /
 * |  /
 * | /
 * P2
 */
void gen_triangle_write (FILE * outf, struct Triangle tri);

/**
 * P1---P3
 * |   / |
 * |  /  |
 * | /   |
 * P2---P4
 */
void gen_rectangle_write (FILE * outf, struct Rectangle rect, unsigned ndivs);

/**
 *    P1------P3  <-- Top rectangle
 *   /|      /|
 *  / |     / |
 * P2-|----P4 |
 * |  |    |  |
 * |  P5---|--P7  <-- Bottom rectangle
 * | /     | /
 * P6------P8
 */
void gen_box_write (FILE * outf, struct Box box, unsigned ndivs);

void gen_cone_write (FILE * outf, struct Cone c);
void gen_cylinder_write (FILE * outf, struct Cylinder c);
void gen_sphere_write (FILE * outf, struct Sphere sph);

/**
 *         |y
 *         |
 *         |   /
 *         |  /
 *       P1|-/---P3
 *      /  |/   /
 * --------|----------> x
 *    /   /   /d
 *  P2---/--P4
 *      / w
 *     /
 *    /z
 *
 * w = d(P1, P3) = d(P2, P4)
 * d = d(P1, P2) = d(P3, P4)
 */
struct Rectangle gen_rectangle_from_wd (float width, float depth);

/**
 *    P1------P3  <-- Top rectangle
 *   /|      /|
 *  / |     / | h
 * P2-|----P4 |
 * |  |    |  |
 * |  P5---|--P7  <-- Bottom rectangle
 * | /     | / d
 * P6------P8
 *     w
 */
struct Box gen_box_from_whd (float width, float height, float depth);

struct Point gen_point_read (FILE * inf);

struct Point Point (float x, float y, float z);
struct Triangle Triangle (struct Point P1, struct Point P2, struct Point P3);
struct Rectangle Rectangle (struct Point P1, struct Point P2, struct Point P3, struct Point P4);
struct Box Box (struct Rectangle top, struct Rectangle bottom);
struct Cone Cone (float rad, float height, unsigned ndivs);
struct Cylinder Cylinder (float rad, float height, unsigned ndivs);
struct Sphere Sphere (float rad, unsigned slices, unsigned stacks);

#endif /* _GENERATORS_H */
