/**
 * Graphical Primitive Generator (Figure Generator)
 * Last Updated: 22-02-2019
 */

#include "generators.h"

#define _USE_MATH_DEFINES
#include <math.h>

/*
 * Internal Functions
 */

/**
 * @brief Performs the scalar operation on a point.
 * @param s - Scalar value.
 * @param A - Point.
 * @returns Returns the point with the X, Y and Z axis values scaled by s.
 */
static inline struct Point operator* (float s, struct Point A) 
{
    return Point(A.x * s, A.y * s, A.z * s);
}

/**
 * @brief Performs the scalar operation on a point.
 * @param s - Scalar value.
 * @param A - Point.
 * @returns Returns the point with the X, Y and Z axis values scaled by s.
 */
static inline struct Point operator* (struct Point A, float s)
{
    return s * A;
}

/**
 * @brief Performs the addition of two points.
 * @param A - First point.
 * @param B - Second point.
 * @returns Returns the result of adding both input points .
 */
static inline struct Point operator+ (struct Point A, struct Point B)
{
    return Point(A.x + B.x, A.y + B.y, A.z + B.z);
}

/**
 * @brief Calculates the norm of a point.
 * @param v - Given point.
 * @returns Returns the result of the operation.
 */
static inline float norm (struct Point v)
{
    return sqrt(v.x * v.x + v.y * v.y + v.z * v.z);
}

/**
 * @brief Calculates the distance between two points.
 * @param A - First point.
 * @param B - Second point.
 * @returns Returns the distance between the two given points. 
 */
static inline float dist (struct Point A, struct Point B)
{
    return norm((-1 * A) + B);
}

/**
 * @brief Operation for normalizing a point.
 * @param A - Input point.
 * @returns Returns the result of normalizing the input point.
 *
 */
static inline struct Point normalize (struct Point A)
{
    return 1 / norm(A) * A;
}

// Already documented in generators.h // 

static inline void gen_point_write_intern (FILE * outf, struct Point p)
{
    fprintf(outf, "%f %f %f\n", p.x, p.y, p.z);
}

static void gen_triangle_write_intern (FILE * outf, struct Triangle tri)
{
    gen_point_write_intern(outf, tri.P1);
    gen_point_write_intern(outf, tri.P2);
    gen_point_write_intern(outf, tri.P3);
}

static void gen_rectangle_write_nodivs_intern (FILE * outf, struct Rectangle rect)
{
    gen_triangle_write_intern(outf, Triangle(rect.P1, rect.P2, rect.P3));
    gen_triangle_write_intern(outf, Triangle(rect.P3, rect.P2, rect.P4));
}

/**
 * P1 ---- P13 ---- P3
 * |        |        |
 * |   R1   |   R3   |
 * |        |        |
 * P12 ---- PM ---- P34
 * |        |        |
 * |   R2   |   R4   |
 * |        |        |
 * P2 ---- P24 ---- P4
 */
static void gen_rectangle_write_intern (FILE * outf, struct Rectangle rect, unsigned ndivs)
{
    const struct Point vw = normalize(rect.P3 + (-1 * rect.P1));
    const struct Point vh = normalize(rect.P2 + (-1 * rect.P1));
    const float w = dist(rect.P3, rect.P1) / (float) ndivs;
    const float h = dist(rect.P2, rect.P1) / (float) ndivs;

    for (unsigned i = 1; i <= ndivs; i++) {
        for (unsigned j = 1; j <= ndivs; j++) {
            struct Point P1 = rect.P1 + (((float) (i - 1) * w) * vw) + (((float) (j - 1) * h) * vh);
            struct Point P2 = rect.P1 + (((float) (i - 1) * w) * vw) + (((float)  j      * h) * vh);
            struct Point P3 = rect.P1 + (((float)  i      * w) * vw) + (((float) (j - 1) * h) * vh);
            struct Point P4 = rect.P1 + (((float)  i      * w) * vw) + (((float)  j      * h) * vh);

            gen_rectangle_write_nodivs_intern(outf, Rectangle(P1, P2, P3, P4));
        }
    }
}

static void gen_box_write_intern (FILE * outf, struct Box box, unsigned ndivs)
{
#   define p1 box.top.P1
#   define p2 box.top.P2
#   define p3 box.top.P3
#   define p4 box.top.P4
#   define p5 box.bottom.P1
#   define p6 box.bottom.P2
#   define p7 box.bottom.P3
#   define p8 box.bottom.P4

    gen_rectangle_write_intern(outf, Rectangle(p1, p5, p2, p6), ndivs); /* Back Left */
    gen_rectangle_write_intern(outf, Rectangle(p3, p7, p1, p5), ndivs); /* Back Right */
    gen_rectangle_write_intern(outf, Rectangle(p7, p8, p5, p6), ndivs); /* Base */

    gen_rectangle_write_intern(outf, Rectangle(p2, p6, p4, p8), ndivs); /* Front Left */
    gen_rectangle_write_intern(outf, Rectangle(p4, p8, p3, p7), ndivs); /* Front Right */
    gen_rectangle_write_intern(outf, box.top, ndivs);
}

static void gen_cone_write_intern (FILE * outf, struct Cone c)
{
    float a = (float) ((2 * M_PI) / c.slices);
    struct Point O = Point(0, 0, 0);
    struct Point C = Point(0, c.height, 0);

    for (unsigned i = 0; i < c.slices; i++) {
        float _i = (float) i;
        float xi = c.rad * sin(_i * a);
        float zi = c.rad * cos(_i * a);

        float xi1 = c.rad * sin((_i + 1) * a);
        float zi1 = c.rad * cos((_i + 1) * a);

        struct Point Pi = Point(xi, 0, zi);
        struct Point Pi1 = Point(xi1, 0, zi1);

        struct Triangle Bi = Triangle(Pi, O, Pi1);
        struct Triangle Si = Triangle(C, Pi, Pi1);

        gen_triangle_write_intern(outf, Bi);
        gen_triangle_write_intern(outf, Si);
    }
}

static void gen_cylinder_write_intern (FILE * outf, struct Cylinder c)
{
    float a = (float) ((2 * M_PI) / c.slices);

    struct Point O = Point(0, -c.height / 2, 0);
    struct Point C = Point(0,  c.height / 2, 0);

    for (unsigned i = 0; i < c.slices; i++) {
        float _i = (float) i;
        float xi = c.rad * sin(_i * a);
        float zi = c.rad * cos(_i * a);

        float xi1 = c.rad * sin((_i + 1) * a);
        float zi1 = c.rad * cos((_i + 1) * a);

        struct Point PiB  = Point(xi,  -c.height / 2, zi);
        struct Point Pi1B = Point(xi1, -c.height / 2, zi1);

        struct Point PiT  = Point(xi,  c.height / 2, zi);
        struct Point Pi1T = Point(xi1, c.height / 2, zi1);

        struct Triangle Bi = Triangle(PiB, O, Pi1B);
        struct Triangle Ti = Triangle(C, PiT, Pi1T);

        gen_triangle_write_intern(outf, Bi);

        for (unsigned j = 0; j < c.stacks; j++) {
            const float dh = c.height / (float) c.stacks;
            const float y13 = (float) (j + 1) * dh;
            const float y24 = (float) j * dh;
            struct Point P1 = PiB  + Point(0, y13, 0);
            struct Point P2 = PiB  + Point(0, y24, 0);
            struct Point P3 = Pi1B + Point(0, y13, 0);
            struct Point P4 = Pi1B + Point(0, y24, 0);

            gen_rectangle_write_nodivs_intern(outf, Rectangle(P1, P2, P3, P4));
        }

        gen_triangle_write_intern(outf, Ti);
    }
}

static void gen_sphere_write_intern (FILE * outf, struct Sphere sph)
{
    std::vector<struct Point> verts;
    verts.reserve((sph.slices + 1) * (sph.stacks + 1));

    for (unsigned i = 0; i <= sph.stacks; i++) {
	/* Stacks range between 0 and 180 degrees (pi).
         * lat represents the current stack step (limited by the total number of stacks)
         */ 
        double lat = ((double) i) / ((double) sph.stacks) * M_PI;

        for (unsigned j = 0; j <= sph.slices; j++) {
            /* Slices range between 0 and 360 degrees (2 * pi).
             * lon represents the current slice step (limited by the total number of slices)
             */
            double lon = ((double) j) / ((double) sph.slices) * 2 * M_PI;

            /* Knowing the latitude and longitude (lat and lon, resp.) we can calculate X, Y and Z as follows:
             * X = r * cos(lon) * sin(lat)
             * Y = r * cos(lat)
             * Z = r * sin(lon) * sin(lat)
             */
            double clat = cos(lat);
            double clon = cos(lon);
            double slat = sin(lat);
            double slon = sin(lon);

            float x = (float) (sph.rad * clon * slat);
            float y = (float) (sph.rad * clat);
            float z = (float) (sph.rad * slon * slat);

            verts.push_back(Point(x, y, z));
        }
    }

    /* Draws the sphere. */
    unsigned len = sph.slices * sph.stacks + sph.slices;
    for (unsigned i = 0; i < len; i++) {
        struct Point P1 = verts[i];
        struct Point P2 = verts[i + sph.slices + 1];
        struct Point P3 = verts[i + sph.slices];
        struct Point P4 = verts[i + 1];

        gen_triangle_write_intern(outf, Triangle(P1, P2, P3));
        gen_triangle_write_intern(outf, Triangle(P2, P1, P4));
    }
}

/**
 * @brief Writing functions.
 */

#define gen_write(Fig, fig, id) \
    void gen_ ## fig ## _write (FILE * outf, struct Fig fig) { \
        fprintf(outf, id);                                     \
        gen_ ## fig ## _write_intern(outf, fig);               \
    } void gen_ ## fig ## _write (FILE * outf, struct Fig fig)

#define gen_write_divs(Fig, fig, id) \
    void gen_ ## fig ## _write (FILE * outf, struct Fig fig, unsigned ndivs) { \
        fprintf(outf, id);                                                     \
        gen_ ## fig ## _write_intern(outf, fig, ndivs);                        \
    } void gen_ ## fig ## _write (FILE * outf, struct Fig fig, unsigned ndivs)

gen_write(     Triangle,  triangle,  "triangle\n");
gen_write_divs(Rectangle, rectangle, "rectangle\n");
gen_write_divs(Box,       box,       "box\n");
gen_write(     Cone,      cone,      "cone\n");
gen_write(     Cylinder,  cylinder,  "cylinder\n");
gen_write(     Sphere,    sphere,    "sphere\n");

/**
 * @brief Reading Functions.
 */

void gen_model_read (FILE * inf, std::vector<struct Point> * vec)
{
    char line[1024] = "";
    fgets(line, 1024, inf); /* Ignore first line */

    struct Point pt = Point(0, 0, 0);
    while (gen_point_read(inf, &pt) != EOF) {
        vec->push_back(pt);
        pt = Point(0, 0, 0);
    }
}

/**
 * @brief Utility functions.
 */

struct Rectangle gen_rectangle_from_wd (float width, float depth)
{
    float w = width / 2;
    float d = depth / 2;

    return Rectangle(
            Point(-w, 0, -d),
            Point(-w, 0,  d),
            Point( w, 0, -d),
            Point( w, 0,  d)
            );
}

struct Box gen_box_from_whd (float width, float height, float depth)
{
    /* Center at (0, 0, 0) */
    float w = width / 2;
    float h = height / 2;
    float d = depth / 2;

    return Box(
            Rectangle(
                Point(-w, h, -d),
                Point(-w, h,  d),
                Point( w, h, -d),
                Point( w, h,  d)
                ),
            Rectangle(
                Point(-w, -h, -d),
                Point(-w, -h,  d),
                Point( w, -h, -d),
                Point( w, -h,  d)
                )
            );
}

int gen_point_read (FILE * inf, struct Point * pt)
{
    return fscanf(inf, "%f %f %f\n", &pt->x, &pt->y, &pt->z);
}

struct Point Point (float x, float y, float z)
{
    struct Point ret;
    ret.x = x;
    ret.y = y;
    ret.z = z;
    return ret;
}

struct Triangle Triangle (struct Point P1, struct Point P2, struct Point P3)
{
    struct Triangle ret;
    ret.P1 = P1;
    ret.P2 = P2;
    ret.P3 = P3;
    return ret;
}

struct Rectangle Rectangle (struct Point P1, struct Point P2, struct Point P3, struct Point P4)
{
    struct Rectangle ret;
    ret.P1 = P1;
    ret.P2 = P2;
    ret.P3 = P3;
    ret.P4 = P4;
    return ret;
}

struct Box Box (struct Rectangle top, struct Rectangle bottom)
{
    struct Box ret;
    ret.top = top;
    ret.bottom = bottom;
    return ret;
}

struct Cone Cone (float rad, float height, unsigned slices)
{
    struct Cone ret;
    ret.rad = rad;
    ret.height = height;
    ret.slices = slices;
    return ret;
}

struct Cylinder Cylinder (float rad, float height, unsigned slices, unsigned stacks)
{
    struct Cylinder ret;
    ret.rad = rad;
    ret.height = height;
    ret.slices = slices;
    ret.stacks = stacks;
    return ret;
}

struct Sphere Sphere (float rad, unsigned slices, unsigned stacks)
{
    struct Sphere ret;
    ret.rad = rad;
    ret.slices = slices;
    ret.stacks = stacks;
    return ret;
}
