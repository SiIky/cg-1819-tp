/**
 * Graphical Primitive Generator (Figure Generator)
 * Last Updated: 22-02-2019
 */

#include "generators.h"

#define _USE_MATH_DEFINES
#include <math.h>

#include <assert.h>
#include <string.h>

#define UNIMPLEMENTED() assert(!"unimplemented")

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

static inline struct Point operator- (struct Point A, struct Point B)
{
    return Point(A.x - B.x, A.y - B.y, A.z - B.z);
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
    return norm(B - A);
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

static inline struct Point normal (struct Point p1, struct Point p2)
{
    return normalize(Point(p1.y * p2.z, p1.z * p2.x, p1.x * p2.y));
}

static struct Point mat_dot_product_MP (const float M[4][4], const struct Point P[4][4], unsigned i, unsigned j)
{
    struct Point ret = Point(0, 0, 0);
    for (unsigned I = 0; I < 4; I++)
        ret = ret + M[i][I] * P[I][j];
    return ret;
}

static struct Point mat_dot_product_PM (const struct Point P[4][4], const float M[4][4], unsigned i, unsigned j)
{
    struct Point ret = Point(0, 0, 0);
    for (unsigned I = 0; I < 4; I++)
        ret = ret + M[I][j] * P[i][I];
    return ret;
}

static void mult_MP (const float M[4][4], const struct Point P[4][4], struct Point r[4][4])
{
    for (unsigned i = 0; i < 4; i++)
        for (unsigned j = 0; j < 4; j++)
            r[i][j] = mat_dot_product_MP(M, P, i, j);
}

static void mult_PM (const struct Point P[4][4], const float M[4][4], struct Point r[4][4])
{
    for (unsigned i = 0; i < 4; i++)
        for (unsigned j = 0; j < 4; j++)
            r[i][j] = mat_dot_product_PM(P, M, i, j);
}

static void mult_MPM (const float M[4][4], const struct Point P[4][4], struct Point r[4][4])
{
    struct Point tmp[4][4];
    mult_MP(M, P, tmp);
    mult_PM(tmp, M, r);
}

static inline void gen_point_write (FILE * outf, struct Point p, struct Point norm)
{
    fprintf(outf,
            "%a %a %a "
            "%a %a %a\n",
            p.x, p.y, p.z,
            norm.x, norm.y, norm.z
           );
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
static void gen_rectangle_write_nodivs (FILE * outf, struct Rectangle rect, struct Rectangle norm)
{
    gen_triangle_write(outf, Triangle(rect.P1, rect.P2, rect.P3), Triangle(norm.P1, norm.P2, norm.P3));
    gen_triangle_write(outf, Triangle(rect.P3, rect.P2, rect.P4), Triangle(norm.P3, norm.P2, norm.P4));
}

static struct Point gen_bezier_get_single_point (const struct Point MPM[4][4], float u, float v, struct Point * N)
{
    struct Point tmp[4];

    for (unsigned j = 0; j < 4; j++)
        tmp[j] = (3 * u * u * MPM[j][0])
            + (2 * u * MPM[j][1])
            + MPM[j][2];
    struct Point Nu = (v * v * v * tmp[0])
        + (v * v * tmp[1])
        + (v * tmp[2])
        + tmp[3];

    for (unsigned j = 0; j < 4; j++)
        tmp[j] = (u * u * u * MPM[j][0])
            + (u * u * MPM[j][1])
            + (u * MPM[j][2])
            + MPM[j][3];
    struct Point Nv = (3 * v * v * tmp[0])
        + (2 * v * tmp[1])
        + tmp[2];

    *N = normal(Nu, Nv);

    for (unsigned j = 0; j < 4; j++)
        tmp[j] = (u * u * u * MPM[j][0])
            + (u * u * MPM[j][1])
            + (u * MPM[j][2])
            + MPM[j][3];
    return (v * v * v * tmp[0])
        + (v * v * tmp[1])
        + (v * tmp[2])
        + tmp[3];
}

static void gen_bezier_patch_read (FILE * inf, std::vector<struct Point> * cps, std::vector<std::vector<unsigned>> * patches)
{
    /* # of patches to read */
    unsigned npatches = 0;
    fscanf(inf, "%u\n", &npatches);
    patches->reserve(npatches);

    /* read the patches */
    for (unsigned p = 0; p < npatches; p++) {
        std::vector<unsigned> patch;
        patch.reserve(16);

        unsigned idx = 0;
        for (unsigned i = 0; i < 15; i++) {
            fscanf(inf, "%u, ", &idx);
            patch.push_back(idx);
        }

        fscanf(inf, "%u\n", &idx);
        patch.push_back(idx);

        patches->push_back(patch);
    }

    /* # of control points */
    unsigned ncps = 0;
    fscanf(inf, "%u\n", &ncps);
    cps->reserve(ncps);

    /* read the control points */
    for (unsigned cp = 0; cp < ncps; cp++) {
        struct Point pt = Point(0, 0, 0);
        fscanf(inf, "%f, %f, %f\n", &pt.x, &pt.y, &pt.z);
        cps->push_back(pt);
    }
}

static void gen_bezier_patch_single (FILE * outf, std::vector<struct Point> cps, std::vector<unsigned> idxs, unsigned tessellation)
{
    const float M[4][4] = {
        { -1,  3, -3, 1, },
        {  3, -6,  3, 0, },
        { -3,  3,  0, 0, },
        {  1,  0,  0, 0, },
    };

    const struct Point P[4][4] = {
        { cps[idxs[0]],  cps[idxs[1]],  cps[idxs[2]],  cps[idxs[3]],  },
        { cps[idxs[4]],  cps[idxs[5]],  cps[idxs[6]],  cps[idxs[7]],  },
        { cps[idxs[8]],  cps[idxs[9]],  cps[idxs[10]], cps[idxs[11]], },
        { cps[idxs[12]], cps[idxs[13]], cps[idxs[14]], cps[idxs[15]], },
    };

    struct Point p = Point(0, 0, 0);
    struct Point MPM[4][4];
    mult_MPM(M, P, MPM);

    for (unsigned i = 1; i <= 4 * tessellation; i++) {
        float u  = ((float) i)     / (4.0 * tessellation);
        float u_ = ((float) i - 1) / (4.0 * tessellation);

        for (unsigned j = 1; j <= 4 * tessellation; j++) {
            float v  = ((float) j)     / (4.0 * tessellation);
            float v_ = ((float) j - 1) / (4.0 * tessellation);

            struct Point N1 = Point(0, 0, 0);
            struct Point N2 = Point(0, 0, 0);
            struct Point N3 = Point(0, 0, 0);
            struct Point N4 = Point(0, 0, 0);

            struct Point P1 = gen_bezier_get_single_point(MPM, u,  v_, &N1);
            struct Point P2 = gen_bezier_get_single_point(MPM, u,  v, &N2);
            struct Point P3 = gen_bezier_get_single_point(MPM, u_, v_, &N3);
            struct Point P4 = gen_bezier_get_single_point(MPM, u_, v, &N4);

            struct Rectangle R = Rectangle(P1, P2, P3, P4);
            struct Rectangle N = Rectangle(N1, N2, N3, N4);
            gen_rectangle_write_nodivs(outf, R, N);
        }
    }
}

void gen_triangle_write (FILE * outf, struct Triangle tri, struct Triangle norms)
{
    gen_point_write(outf, tri.P1, norms.P1);
    gen_point_write(outf, tri.P2, norms.P2);
    gen_point_write(outf, tri.P3, norms.P3);
}

void gen_rectangle_write (FILE * outf, struct Rectangle rect, unsigned ndivs)
{
    const struct Point vw = normalize(rect.P3 - rect.P1);
    const struct Point vh = normalize(rect.P2 - rect.P1);
    const struct Point N = normal(vw, vh);
    const float w = dist(rect.P3, rect.P1) / (float) ndivs;
    const float h = dist(rect.P2, rect.P1) / (float) ndivs;

    for (unsigned i = 1; i <= ndivs; i++) {
        for (unsigned j = 1; j <= ndivs; j++) {
            struct Point P1 = rect.P1 + (((float) (i - 1) * w) * vw) + (((float) (j - 1) * h) * vh);
            struct Point P2 = rect.P1 + (((float) (i - 1) * w) * vw) + (((float)  j      * h) * vh);
            struct Point P3 = rect.P1 + (((float)  i      * w) * vw) + (((float) (j - 1) * h) * vh);
            struct Point P4 = rect.P1 + (((float)  i      * w) * vw) + (((float)  j      * h) * vh);

            gen_rectangle_write_nodivs(outf, Rectangle(P1, P2, P3, P4), Rectangle(N, N, N, N));
        }
    }
}

void gen_box_write (FILE * outf, struct Box box, unsigned ndivs)
{
#   define p1 box.top.P1
#   define p2 box.top.P2
#   define p3 box.top.P3
#   define p4 box.top.P4
#   define p5 box.bottom.P1
#   define p6 box.bottom.P2
#   define p7 box.bottom.P3
#   define p8 box.bottom.P4

    gen_rectangle_write(outf, Rectangle(p1, p5, p2, p6), ndivs); /* Back Left */
    gen_rectangle_write(outf, Rectangle(p3, p7, p1, p5), ndivs); /* Back Right */
    gen_rectangle_write(outf, Rectangle(p7, p8, p5, p6), ndivs); /* Base */

    gen_rectangle_write(outf, Rectangle(p2, p6, p4, p8), ndivs); /* Front Left */
    gen_rectangle_write(outf, Rectangle(p4, p8, p3, p7), ndivs); /* Front Right */
    gen_rectangle_write(outf, box.top, ndivs);
}

/*
 * i -> line/stack
 * j -> slice
 *
 *       ^
 *      /|\
 *     / | \
 *    /  |  \    <- Pij = (ri * sin(2j*pi/slices), h - , ri * cos(2j*pi/slices))
 *   /  h|   \
 *  /    |    \
 * ------+------
 *    r
 */
void gen_cone_write (FILE * outf, struct Cone c)
{
    const float a = (float) ((2 * M_PI) / (float) c.slices);
    const struct Point O = Point(0, 0, 0);
    const struct Point C = Point(0, c.height, 0);
    const float st = (float) c.stacks;

    for (unsigned i = 0; i < c.slices; i++) {
        const float I = (float) i;

        const float xi = sin(I * a);
        const float zi = cos(I * a);

        const float xi1 = sin((I + 1) * a);
        const float zi1 = cos((I + 1) * a);

        struct Point Nsi = Point(0, 0, 0);
        {
            /* side vector */
            struct Point S = C - (c.rad * Point(xi, 0, zi));
            /* XZ plane vector */
            struct Point B = c.rad * Point(sin((I * a) + (M_PI / 2)), 0, cos((I * a) + (M_PI / 2)));
            Nsi = normal(B, S);
        }

        struct Point Nsi1 = Point(0, 0, 0);
        {
            /* side vector */
            struct Point S = C - (c.rad * Point(xi1, 0, zi1));
            /* XZ plane vector */
            struct Point B = c.rad * Point(sin(((I + 1) * a) + (M_PI / 2)), 0, cos(((I + 1) * a) + (M_PI / 2)));
            Nsi1 = normal(B, S);
        }

        /* draw top mini cone */
        {
            const float R = c.rad / st;
            const float H = c.height * (st - 1) / st;

            struct Point P2 = R * Point(xi, H, zi);
            struct Point P3 = R * Point(xi1, H, zi1);
            struct Point N = normal(P2 - C, P3 - C);

            gen_triangle_write(outf, Triangle(C, P2, P3), Triangle(N, N, N));
        }

        /* draw base */
        {
            struct Point P1 = c.rad * Point(xi, 0, zi);
            struct Point P2 = Point(0, 0, 0);
            struct Point P3 = c.rad * Point(xi1, 0, zi1);
            const struct Point N = Point(0, -1, 0);

            gen_triangle_write(outf, Triangle(P1, P2, P3), Triangle(N, N, N));
        }

        /* draw side */
        for (unsigned j = 0; j < (c.stacks - 1); j++) {
            const float J = (float) j;

            const float y  = c.height * (J       / st);
            const float y1 = c.height * ((J + 1) / st);

            const float r  = c.rad * (st - J)     / st;
            const float r1 = c.rad * (st - J - 1) / st;

            const struct Point P1 = Point(r1 * sin(I       * a),  y1, r1 * cos(I       * a));
            const struct Point P2 = Point(r  * sin(I       * a),  y,  r  * cos(I       * a));
            const struct Point P3 = Point(r1 * sin((I + 1) * a),  y1, r1 * cos((I + 1) * a));
            const struct Point P4 = Point(r  * sin((I + 1) * a),  y,  r  * cos((I + 1) * a));
            const struct Point N12 = normal(P1 - P2, P4 - P3);
            const struct Point N34 = normal(P1 - P2, P4 - P3);

            gen_rectangle_write_nodivs(outf, Rectangle(P1, P2, P3, P4), Rectangle(Nsi, Nsi, Nsi1, Nsi1));
        }
    }
}

void gen_cylinder_write (FILE * outf, struct Cylinder c)
{
    float a = (float) ((2 * M_PI) / c.slices);

    struct Point O = Point(0, -c.height / 2, 0);
    struct Point C = Point(0,  c.height / 2, 0);

    for (unsigned i = 0; i < c.slices; i++) {
        float I = (float) i;
        float xi = c.rad * sin(I * a);
        float zi = c.rad * cos(I * a);

        float xi1 = c.rad * sin((I + 1) * a);
        float zi1 = c.rad * cos((I + 1) * a);

        struct Point PiB  = Point(xi,  -c.height / 2, zi);
        struct Point Pi1B = Point(xi1, -c.height / 2, zi1);

        struct Point PiT  = Point(xi,  c.height / 2, zi);
        struct Point Pi1T = Point(xi1, c.height / 2, zi1);

        struct Triangle Bi = Triangle(PiB, O, Pi1B);
        struct Triangle Ti = Triangle(C, PiT, Pi1T);
        struct Point N = Point(0, -1, 0);

        gen_triangle_write(outf, Bi, Triangle(N, N, N));

        for (unsigned j = 0; j < c.stacks; j++) {
            const float dh = c.height / (float) c.stacks;
            const float y13 = (float) (j + 1) * dh;
            const float y24 = (float) j * dh;
            struct Point P1 = PiB  + Point(0, y13, 0);
            struct Point P2 = PiB  + Point(0, y24, 0);
            struct Point P3 = Pi1B + Point(0, y13, 0);
            struct Point P4 = Pi1B + Point(0, y24, 0);

            struct Point Ni = normalize(Point(PiB.x, 0, PiB.z));
            struct Point Ni1 = normalize(Point(Pi1B.x, 0, Pi1B.z));
            gen_rectangle_write_nodivs(outf,
                    Rectangle(P1, P2, P3, P4),
                    Rectangle(Ni, Ni, Ni1, Ni1));
        }

        N = Point(0, 1, 0);
        gen_triangle_write(outf, Ti, Triangle(N, N, N));
    }
}

void gen_sphere_write (FILE * outf, struct Sphere sph)
{
    std::vector<struct Point> verts;
    verts.reserve((sph.slices + 1) * (sph.stacks + 1));
    std::vector<struct Point> normals;
    normals.reserve((sph.slices + 1) * (sph.stacks + 1));

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
            normals.push_back(normalize(Point(x, y, z)));
        }
    }

    /* Draws the sphere. */
    unsigned len = sph.slices * sph.stacks + sph.slices;
    for (unsigned i = 0; i < len; i++) {
        struct Point P1 = verts[i];
        struct Point P2 = verts[i + sph.slices + 1];
        struct Point P3 = verts[i + sph.slices];
        struct Point P4 = verts[i + 1];

        struct Point N1 = normals[i];
        struct Point N2 = normals[i + sph.slices + 1];
        struct Point N3 = normals[i + sph.slices];
        struct Point N4 = normals[i + 1];

        gen_triangle_write(outf, Triangle(P1, P2, P3), Triangle(N1, N2, N3));
        gen_triangle_write(outf, Triangle(P2, P1, P4), Triangle(N2, N1, N4));
    }
}

void gen_bezier_patch_write (FILE * outf, FILE * inf, unsigned tessellation)
{
    std::vector<struct Point> cps;
    std::vector<std::vector<unsigned>> patches;
    gen_bezier_patch_read(inf, &cps, &patches);
    for (std::vector<unsigned> patch : patches)
        gen_bezier_patch_single(outf, cps, patch, tessellation);
}

/**
 * Reading Functions.
 */

void gen_model_read (FILE * inf, std::vector<struct Point> * vec, std::vector<struct Point> * norms)
{
    char line[1024] = "";
    struct Point pt = Point(0, 0, 0);
    struct Point norm = Point(0, 0, 0);
    while (fgets(line, 1024, inf) != NULL && gen_point_read(line, &pt, &norm)) {
        vec->push_back(pt);
        norms->push_back(norm);
        pt = Point(0, 0, 0);
        norm = Point(0, 0, 0);
    }
}

/**
 * Utility functions.
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

bool gen_point_read (char * line, struct Point * pt, struct Point * norm)
{
    return sscanf(line,
            "%f %f %f "
            "%f %f %f\n",
            &pt->x, &pt->y, &pt->z,
            &norm->x, &norm->y, &norm->z
            ) == 6;
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

struct Cone Cone (float rad, float height, unsigned slices, unsigned stacks)
{
    struct Cone ret;
    ret.rad = rad;
    ret.height = height;
    ret.slices = slices;
    ret.stacks = stacks;
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
