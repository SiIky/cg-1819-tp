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

struct Square {
    struct Point P1;
    struct Point P2;
    struct Point P3;
    struct Point P4;
};

struct Cube {
    struct Square top;
    struct Square bottom;
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
void gen_square_write (FILE * outf, struct Square sq);

/**
 *    P1------P3  <-- Top square
 *   /|      /|
 *  / |     / |
 * P2-|----P4 |
 * |  |    |  |
 * |  P5---|--P7  <-- Bottom square
 * | /     | /
 * P6------P8
 */
void gen_cube_write (FILE * outf, struct Cube cb);

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
struct Square gen_square_from_wd (float width, float depth);

/**
 *    P1------P3  <-- Top square
 *   /|      /|
 *  / |     / | h
 * P2-|----P4 |
 * |  |    |  |
 * |  P5---|--P7  <-- Bottom square
 * | /     | / d
 * P6------P8
 *     w
 */
struct Cube gen_cube_from_whd (float width, float height, float depth);

#define Point(X, Y, Z)           ((struct Point)    { .x = (X), .y = (Y), .z = (Z), })
#define Triangle(p1, p2, p3)     ((struct Triangle) { .P1 = (p1), .P2 = (p2), .P3 = (p3), })
#define Square(  p1, p2, p3, p4) ((struct Square)   { .P1 = (p1), .P2 = (p2), .P3 = (p3), .P4 = (p4), })
#define Cube(TOP, BOTTOM)        ((struct Cube)     { .top = (TOP), .bottom = (BOTTOM), })

#endif /* _GENERATORS_H */
