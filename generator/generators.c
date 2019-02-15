#include "generators.h"

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

static void gen_square_write_intern (FILE * outf, struct Square sq)
{
    gen_triangle_write_intern(outf, Triangle(sq.P1, sq.P2, sq.P3));
    gen_triangle_write_intern(outf, Triangle(sq.P3, sq.P2, sq.P4));
}

static void gen_cube_write_intern (FILE * outf, struct Cube cb)
{
#define p1 cb.top.P1
#define p2 cb.top.P2
#define p3 cb.top.P3
#define p4 cb.top.P4
#define p5 cb.bottom.P1
#define p6 cb.bottom.P2
#define p7 cb.bottom.P3
#define p8 cb.bottom.P4

    gen_square_write_intern(outf, Square(p1, p5, p2, p6)); /* Back Left */
    gen_square_write_intern(outf, Square(p1, p5, p3, p7)); /* Back Right */
    gen_square_write_intern(outf, cb.bottom);

    gen_square_write_intern(outf, Square(p2, p6, p4, p8)); /* Front Left */
    gen_square_write_intern(outf, Square(p4, p8, p3, p7)); /* Front Right */
    gen_square_write_intern(outf, cb.top);
}

/*
 * WRITE
 */

#define gen_write(Fig, fig, id) \
    void gen_ ## fig ## _write (FILE * outf, struct Fig fig) { \
        fprintf(outf, id);                                     \
        gen_ ## fig ## _write_intern(outf, fig);               \
    } void gen_ ## fig ## _write (FILE * outf, struct Fig fig)

gen_write(Triangle, triangle, "triangle\n");
gen_write(Square,   square,   "square\n");
gen_write(Cube,     cube,     "cube\n");

/*
 * UTILITY
 */

struct Square gen_square_from_wd (float width, float depth)
{
    float w = width / 2;
    float d = depth / 2;

    return Square(
            Point(-w, 0, -d),
            Point(-w, 0,  d),
            Point( w, 0, -d),
            Point( w, 0,  d)
            );
}

struct Cube gen_cube_from_whd (float width, float height, float depth)
{
    /* Center at (0, 0, 0) */
    float w = width / 2;
    float h = height / 2;
    float d = depth / 2;

    return Cube(
            Square(
                Point(-w, h, -d),
                Point(-w, h,  d),
                Point( w, h, -d),
                Point( w, h,  d)
                ),
            Square(
                Point(-w, -h, -d),
                Point(-w, -h,  d),
                Point( w, -h, -d),
                Point( w, -h,  d)
                )
            );
}

struct Point gen_point_read_intern (FILE * inf)
{
    struct Point ret = {0};
    fscanf(inf, "%f %f %f\n", &ret.x, &ret.y, &ret.z);
    return ret;
}
