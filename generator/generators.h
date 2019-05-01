/*
 * Graphical Primitive Generator (Header File)
 * Last Updated: 22-02-2019 
 */

#ifndef _GENERATORS_H
#define _GENERATORS_H

#include <stdio.h>
#include <stdbool.h>

#include <vector>

/**
 * Representation of a point on the X, Y and Z axis.
 */
struct Point {
    float x;
    float y;
    float z;
};

/**
 * Representation of a triangle as a set of 3 points.
 */
struct Triangle {
    struct Point P1;
    struct Point P2;
    struct Point P3;
};

/**
 * Representation of a rectangle as a set of 4 points.
 */
struct Rectangle {
    struct Point P1;
    struct Point P2;
    struct Point P3;
    struct Point P4;
};

/**
 * Representation of a box as a set of 2 rectangles.
 */
struct Box {
    struct Rectangle top;
    struct Rectangle bottom;
};

/**
 * Representation of a Cone.
 */
struct Cone {
    float rad;          // radius
    float height;
    unsigned slices;
    unsigned stacks;
};

/**
 * Representation of a Cylinder.
 */
struct Cylinder {
    float rad;
    float height;
    unsigned slices;
    unsigned stacks;
};

/**
 * Representation of a Sphere.
 */
struct Sphere {
    float rad;
    unsigned slices;
    unsigned stacks;
};

/**
 * @brief Outputs to a file the result of generating a triangle with the provided values.
 * @param outf - Output file.
 * @param tri - Input triangle.
 *
 * P1---P3
 * |    /
 * |   /
 * |  /
 * | /
 * P2
 */
void gen_triangle_write (FILE * outf, struct Triangle tri);

/**
 * @brief Outputs to a file the result of generating a rectangle with the provided values.
 * @param outf - Output file.
 * @param box - Input rectangle.
 * @param ndivs - Number of divisions.
 *
 * @brief Outputs to a file the result of generating a rectangle with the provided values.
 * @param outf - Output file.
 * @param tri - Input rectangle.
 * @param ndivs - Number of divisions.
 *
 * P1---P3
 * |   / |
 * |  /  |
 * | /   |
 * P2---P4
 */
void gen_rectangle_write (FILE * outf, struct Rectangle rect, unsigned ndivs);

/**
 * @brief Outputs to a file the result of generating a box with the provided values.
 * @param outf - Output file.
 * @param box - Input box.
 * @param ndivs - Number of divisions.
 *
 *    P1------P3  <-- Top rectangle
 *   /|      /|
 *  / |     / |
 * P2------P4 |
 * |  |    |  |
 * |  P5---|--P7  <-- Bottom rectangle
 * | /     | /
 * P6------P8
 */
void gen_box_write (FILE * outf, struct Box box, unsigned ndivs);

/**
 * @brief Outputs to a file the result of generating a cone with the provided values.
 * @param outf - Output file.
 * @param cone - Input cone.
 */
void gen_cone_write (FILE * outf, struct Cone c);

/**
 * @brief Outputs to a file the result of generating a cylinder with the provided values.
 * @param outf - Output file.
 * @param box - Input cylinder.
 */
void gen_cylinder_write (FILE * outf, struct Cylinder c);

/**
 * @brief Outputs to a file the result of generating a sphere with the provided values.
 * @param outf - Output file.
 * @param box - Input sphere.
 */
void gen_sphere_write (FILE * outf, struct Sphere sph);

void gen_bezier_patch_write (FILE * outf, FILE * inf, unsigned tessellation);

/**
 * @brief Generates a Rectangle from width-depth.
 *
 *
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
 * @param width - w = d(P1, P3) = d(P2, P4).
 * @param depth - d = d(P1, P2) = d(P3, P4).
 */
struct Rectangle gen_rectangle_from_wd (float width, float depth);

/**
 * @brief Generates a box from width-height-depth.
 *
 *
 *    P1------P3  <-- Top rectangle
 *   /|      /|
 *  / |     / | h
 * P2-|----P4 |
 * |  |    |  |
 * |  P5---|--P7  <-- Bottom rectangle
 * | /     | / d
 * P6------P8
 *     w
 *
 * @param width - Width value.
 * @param height - Height value.
 * @param depth - Depth value.
 */
struct Box gen_box_from_whd (float width, float height, float depth);

/**
 * @brief Reads a point from a file.
 * @param inf - Input file.
 * @param pt - Point.  
 */
int gen_point_read (char * line, struct Point * pt);

/**
 * @brief Reads a model from a file.
 * @param inf - Input file.
 * @param vec - Vector of points.
 */
void gen_model_read (FILE * inf, std::vector<struct Point> * vec, std::vector<struct Point> * norm);

/* Operations on structs */

/**
 * @brief "Constructor" for a point.
 * @param x - Value on the X axis.
 * @param y - Value on the Y axis.
 * @param z - Value on the Z axis.
 * @returns Generated point by input. 
 */
struct Point Point (float x, float y, float z);

/**
 * @brief "Constructor" for a triangle.
 * @param P1 - Point 1.
 * @param P2 - Point 2.
 * @param P3 - Point 3.
 * @returns Generated triangle by input. 
 */
struct Triangle Triangle (struct Point P1, struct Point P2, struct Point P3);

/**
 * @brief "Constructor" for a rectangle.
 * @param P1 - Point 1.
 * @param P2 - Point 2.
 * @param P3 - Point 3.
 * @param P4 - Point 4. 
 * @returns Generated rectangle by input. 
 */
struct Rectangle Rectangle (struct Point P1, struct Point P2, struct Point P3, struct Point P4);

/**
 * @brief "Constructor" for a box.
 * @param top - Uppermost Rectangle.
 * @param bottom - Bottom Rectangle.
 * @returns Generated box by input. 
 */
struct Box Box (struct Rectangle top, struct Rectangle bottom);

/**
 * @brief "Constructor" for a Cone.
 * @param rad - Radius.
 * @param height - Height.
 * @param slices - Slices.
 * @returns Generated cone by input. 
 */
struct Cone Cone (float rad, float height, unsigned slices, unsigned stacks);

/**
 * @brief "Constructor" for a Cylinder.
 * @param rad - Radius.
 * @param height - Height.
 * @param slices - Slices.
 * @param stacks - Stacks.
 * @returns Generated cylinder by input. 
 */
struct Cylinder Cylinder (float rad, float height, unsigned slices, unsigned stacks);

/**
 * @brief "Constructor" for a Sphere.
 * @param rad - Radius.
 * @param slices - Slices.
 * @param stacks - Stacks.
 * @returns Generated sphere by input. 
 */
struct Sphere Sphere (float rad, unsigned slices, unsigned stacks);

#endif /* _GENERATORS_H */
