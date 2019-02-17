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

static void gen_rectangle_write_intern (FILE * outf, struct Rectangle rect)
{
    gen_triangle_write_intern(outf, Triangle(rect.P1, rect.P2, rect.P3));
    gen_triangle_write_intern(outf, Triangle(rect.P3, rect.P2, rect.P4));
}

static void gen_box_write_intern (FILE * outf, struct Box box)
{
#define p1 box.top.P1
#define p2 box.top.P2
#define p3 box.top.P3
#define p4 box.top.P4
#define p5 box.bottom.P1
#define p6 box.bottom.P2
#define p7 box.bottom.P3
#define p8 box.bottom.P4

    gen_rectangle_write_intern(outf, Rectangle(p1, p5, p2, p6)); /* Back Left */
    gen_rectangle_write_intern(outf, Rectangle(p1, p5, p3, p7)); /* Back Right */
    gen_rectangle_write_intern(outf, box.bottom);

    gen_rectangle_write_intern(outf, Rectangle(p2, p6, p4, p8)); /* Front Left */
    gen_rectangle_write_intern(outf, Rectangle(p4, p8, p3, p7)); /* Front Right */
    gen_rectangle_write_intern(outf, box.top);
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

gen_write(Triangle,  triangle,  "triangle\n");
gen_write(Rectangle, rectangle, "rectangle\n");
gen_write(Box,       box,       "box\n");
gen_write(Sphere,    sphere,    "sphere\n");

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

struct Point gen_point_read_intern (FILE * inf)
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

struct Sphere Sphere (float rad, unsigned slices, unsigned stacks)
{
    struct Sphere ret;
    ret.rad = rad;
    ret.slices = slices;
    ret.stacks = stacks;
    return ret;
}
