#include "generators.h"

#include <math.h>
#include <vector>

/*
 * INTERNAL FUNCTIONS
 */

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
    if (ndivs == 0) {
        gen_triangle_write_intern(outf, Triangle(rect.P1, rect.P2, rect.P3));
        gen_triangle_write_intern(outf, Triangle(rect.P3, rect.P2, rect.P4));
    } else {
        struct Point P1 = rect.P1;
        struct Point P2 = rect.P2;
        struct Point P3 = rect.P3;
        struct Point P4 = rect.P4;

#define MIDPOINT(A, B) Point((A.x + B.x) / 2, (A.y + B.y) / 2, (A.z + B.z) / 2)
        struct Point P13 = MIDPOINT(P1, P3);
        struct Point P12 = MIDPOINT(P1, P2);
        struct Point P24 = MIDPOINT(P2, P4);
        struct Point P34 = MIDPOINT(P3, P4);

        struct Point PM;
        {
            struct Point PM1 = MIDPOINT(P12, P34);
            struct Point PM2 = MIDPOINT(P13, P24);
            struct Point PM12 = MIDPOINT(PM1, PM2);

            struct Point P14 = MIDPOINT(P1, P4);
            struct Point P23 = MIDPOINT(P2, P3);
            struct Point P1423 = MIDPOINT(P14, P23);

            PM = MIDPOINT(PM12, P1423);
        }
#undef MIDPOINT

        struct Rectangle R1 = Rectangle(P1,  P12, P13, PM);
        struct Rectangle R2 = Rectangle(P12, P2,  PM,  P24);
        struct Rectangle R3 = Rectangle(P13, PM,  P3,  P34);
        struct Rectangle R4 = Rectangle(PM,  P24, P34, P4);

        gen_rectangle_write_intern(outf, R1, ndivs - 1);
        gen_rectangle_write_intern(outf, R2, ndivs - 1);
        gen_rectangle_write_intern(outf, R3, ndivs - 1);
        gen_rectangle_write_intern(outf, R4, ndivs - 1);
    }
}

static void gen_box_write_intern (FILE * outf, struct Box box, unsigned ndivs)
{
#define p1 box.top.P1
#define p2 box.top.P2
#define p3 box.top.P3
#define p4 box.top.P4
#define p5 box.bottom.P1
#define p6 box.bottom.P2
#define p7 box.bottom.P3
#define p8 box.bottom.P4

    gen_rectangle_write_intern(outf, Rectangle(p1, p5, p2, p6), ndivs); /* Back Left */
    gen_rectangle_write_intern(outf, Rectangle(p3, p7, p1, p5), ndivs); /* Back Right */
    gen_rectangle_write_intern(outf, Rectangle(p7, p8, p5, p6), ndivs); /* Base */

    gen_rectangle_write_intern(outf, Rectangle(p2, p6, p4, p8), ndivs); /* Front Left */
    gen_rectangle_write_intern(outf, Rectangle(p4, p8, p3, p7), ndivs); /* Front Right */
    gen_rectangle_write_intern(outf, box.top, ndivs);
}

static void gen_cone_write_intern (FILE * outf, struct Cone c)
{
    float a = (float) ((2 * M_PI) / c.ndivs);
    struct Point O = Point(0, 0, 0);
    struct Point C = Point(0, c.height, 0);

    for (unsigned i = 0; i < c.ndivs; i++) {
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
    float a = (float) ((2 * M_PI) / c.ndivs);
    struct Point O = Point(0, 0, 0);
    struct Point C = Point(0, c.height, 0);

    for (unsigned i = 0; i < c.ndivs; i++) {
        float _i = (float) i;
        float xi = c.rad * sin(_i * a);
        float zi = c.rad * cos(_i * a);

        float xi1 = c.rad * sin((_i + 1) * a);
        float zi1 = c.rad * cos((_i + 1) * a);

        struct Point PiB = Point(xi, 0, zi);
        struct Point Pi1B = Point(xi1, 0, zi1);

        struct Point PiT = Point(xi, c.height, zi);
        struct Point Pi1T = Point(xi1, c.height, zi1);

        struct Triangle Bi = Triangle(PiB, O, Pi1B);
        struct Rectangle SR = Rectangle(PiT, PiB, Pi1T, Pi1B);
        struct Triangle Ti = Triangle(C, PiT, Pi1T);

        gen_triangle_write_intern(outf, Bi);
        gen_rectangle_write_intern(outf, SR, 0);
        gen_triangle_write_intern(outf, Ti);
    }
}

static void gen_sphere_write_intern (FILE * outf, struct Sphere sph)
{
    std::vector<struct Point> verts;
    verts.reserve((sph.slices + 1) * (sph.stacks + 1));

    for (unsigned i = 0; i <= sph.stacks; i++) {
        double lat = ((double) i) / ((double) sph.stacks) * M_PI;

        for (unsigned j = 0; j <= sph.slices; j++) {
            double lon = ((double) j) / ((double) sph.slices) * 2 * M_PI;

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

/*
 * WRITE
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

/*
 * UTILITY
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

struct Point gen_point_read (FILE * inf)
{
    struct Point ret = Point(0, 0, 0);
    fscanf(inf, "%f %f %f\n", &ret.x, &ret.y, &ret.z);
    return ret;
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

struct Cone Cone (float rad, float height, unsigned ndivs)
{
    struct Cone ret;
    ret.rad = rad;
    ret.height = height;
    ret.ndivs = ndivs;
    return ret;
}

struct Cylinder Cylinder (float rad, float height, unsigned ndivs)
{
    struct Cylinder ret;
    ret.rad = rad;
    ret.height = height;
    ret.ndivs = ndivs;
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
