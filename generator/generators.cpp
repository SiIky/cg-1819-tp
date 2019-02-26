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

static inline struct Point operator/ (struct Point A, float s)
{
    return Point(A.x / s, A.y / s, A.z / s);
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

static struct Point zipWith_mul (struct Point A, struct Point B)
{
    return Point(A.x * B.x, A.y * B.y, A.z * B.z);
}

static struct Point rep (unsigned k)
{
    static const struct Point ret[] = {
        Point(1,  0,  0),
        Point(-1, 0,  0),
        Point(0,  1,  0),
        Point(0,  -1, 0),
        Point(0,  0,  1),
        Point(0,  0,  -1),
    };
    return ret[k % 6];
}

static void gen_box_write_intern (FILE * outf, struct Box box, unsigned ndivs)
{
#   define p1 box.top.P1
#   define p2 box.top.P2
#   define p3 box.top.P3
#   define p5 box.bottom.P1

    const struct Point C = Point(p3.x - p1.x, p5.y - p1.y, p2.z - p1.z) / 2;

    for (unsigned k = 0; k < 6; k++) {
        const struct Point K  = rep(k);
        const struct Point Ki = rep(k + 2);
        const struct Point Kj = rep(k + 4);

        for (unsigned i = 1; i < ndivs; i++) {
            for (unsigned j = 1; j < ndivs; j++) {
//#define P(i, j) (zipWith_mul(C, K + ((1 - 2 * (i) / ndivs) * Ki) + ((1 - 2 * (j) / ndivs) * Kj)))
#define P(i, j) (zipWith_mul(C, K + (2 * (i) * Ki / ndivs) + (2 * (j) * Kj / ndivs)))
                const struct Point P1 = P(i - 1, j - 1);
                const struct Point P2 = P(i - 1, j);
                const struct Point P3 = P(i,     j - 1);
                const struct Point P4 = P(i,     j);

                gen_rectangle_write_nodivs_intern(outf, Rectangle(P1, P2, P3, P4));
            }
        }
    }
}

static void gen_xmas_tree0_write_intern (FILE * outf, struct Cone c)
{
	float a = (float) ((2 * M_PI) / c.slices);
	struct Point O = Point(0, 0, 0);

	for (unsigned i = 0; i < c.slices; i++) {
		const float I = (float) i;

		/* draw top mini cone */
		{
			const float R = c.rad / c.stacks;
			const float H = c.height * ((c.stacks - 1) / c.stacks);

			const float xi = R * sin(I * a);
			const float zi = R * cos(I * a);

			const float xi1 = R * sin((I + 1) * a);
			const float zi1 = R * cos((I + 1) * a);

			gen_triangle_write_intern(outf, Triangle(
						Point(0, c.height, 0),
						Point(xi, H, zi),
						Point(xi1, H, zi1)));
		}

		/* draw base */
		{
			const float xi = c.rad * sin(I * a);
			const float zi = c.rad * cos(I * a);

			const float xi1 = c.rad * sin((I + 1) * a);
			const float zi1 = c.rad * cos((I + 1) * a);

			gen_triangle_write_intern(outf, Triangle(
						Point(xi,  0, zi),
						Point(0,   0, 0),
						Point(xi1, 0, zi1)));
		}

		/* draw side */
		for (unsigned j = 0; j < c.stacks - 1; j++) {
			const float J = (float) j;

			const float r1 = c.rad * ((c.stacks - J)       / c.stacks);
			const float r  = c.rad * ((c.stacks - (J + 1)) / c.stacks);

			const float y  = c.height * (J / c.stacks);
			const float y1 = c.height * ((J + 1) / c.stacks);

			const struct Point P1 = Point(r1 * sin(I       * a),  y1, r1 * cos(I       * a));
			const struct Point P2 = Point(r  * sin(I       * a),  y,  r  * cos(I       * a));
			const struct Point P3 = Point(r1 * sin((I + 1) * a),  y1, r1 * cos((I + 1) * a));
			const struct Point P4 = Point(r  * sin((I + 1) * a),  y1, r  * cos((I + 1) * a));

			gen_rectangle_write_nodivs_intern(outf, Rectangle(P1, P2, P3, P4));
		}
	}
}

static void gen_xmas_tree1_write_intern (FILE * outf, struct Cone c)
{
	float a = (float) ((2 * M_PI) / c.slices);
	struct Point O = Point(0, 0, 0);

	for (unsigned i = 0; i < c.slices; i++) {
		const float I = (float) i;

		/* draw top mini cone */
		{
			const float R = c.rad / c.stacks;
			const float H = c.height * ((c.stacks - 1) / c.stacks);

			const float xi = R * sin(I * a);
			const float zi = R * cos(I * a);

			const float xi1 = R * sin((I + 1) * a);
			const float zi1 = R * cos((I + 1) * a);

			gen_triangle_write_intern(outf, Triangle(
						Point(0, c.height, 0),
						Point(xi, H, zi),
						Point(xi1, H, zi1)));
		}

		/* draw base */
		{
			const float xi = c.rad * sin(I * a);
			const float zi = c.rad * cos(I * a);

			const float xi1 = c.rad * sin((I + 1) * a);
			const float zi1 = c.rad * cos((I + 1) * a);

			gen_triangle_write_intern(outf, Triangle(
						Point(xi,  0, zi),
						Point(0,   0, 0),
						Point(xi1, 0, zi1)));
		}

		/* draw side */
		for (unsigned j = 0; j < c.stacks - 1; j++) {
			const float J = (float) j;

			const float y  = c.height * (J / c.stacks);
			const float y1 = c.height * ((J + 1) / c.stacks);

			const float r  = c.rad * y;
			const float r1 = c.rad * y1;

			const struct Point P1 = Point(r1 * sin(I       * a),  y1, r1 * cos(I       * a));
			const struct Point P2 = Point(r  * sin(I       * a),  y,  r  * cos(I       * a));
			const struct Point P3 = Point(r1 * sin((I + 1) * a),  y1, r1 * cos((I + 1) * a));
			const struct Point P4 = Point(r  * sin((I + 1) * a),  y1, r  * cos((I + 1) * a));

			gen_rectangle_write_nodivs_intern(outf, Rectangle(P1, P2, P3, P4));
		}
	}
}

static void gen_xmas_tree2_write_intern (FILE * outf, struct Cone c)
{
	float a = (float) ((2 * M_PI) / c.slices);
	struct Point O = Point(0, 0, 0);

	for (unsigned i = 0; i < c.slices; i++) {
		const float I = (float) i;

		/* draw top mini cone */
		{
			const float R = c.rad / c.stacks;
			const float H = c.height * ((c.stacks - 1) / c.stacks);

			const float xi = R * sin(I * a);
			const float zi = R * cos(I * a);

			const float xi1 = R * sin((I + 1) * a);
			const float zi1 = R * cos((I + 1) * a);

			gen_triangle_write_intern(outf, Triangle(
						Point(0, c.height, 0),
						Point(xi, H, zi),
						Point(xi1, H, zi1)));
		}

		/* draw base */
		{
			const float xi = c.rad * sin(I * a);
			const float zi = c.rad * cos(I * a);

			const float xi1 = c.rad * sin((I + 1) * a);
			const float zi1 = c.rad * cos((I + 1) * a);

			gen_triangle_write_intern(outf, Triangle(
						Point(xi,  0, zi),
						Point(0,   0, 0),
						Point(xi1, 0, zi1)));
		}

		/* draw side */
		for (unsigned j = 0; j < c.stacks - 1; j++) {
			const float J = (float) j;

			const float y  = c.height * (J / c.stacks);
			const float y1 = c.height * ((J + 1) / c.stacks);

			const float r  = c.rad * y  / c.height;
			const float r1 = c.rad * y1 / c.height;

			const struct Point P1 = Point(r1 * sin(I       * a),  y1, r1 * cos(I       * a));
			const struct Point P2 = Point(r  * sin(I       * a),  y,  r  * cos(I       * a));
			const struct Point P3 = Point(r1 * sin((I + 1) * a),  y1, r1 * cos((I + 1) * a));
			const struct Point P4 = Point(r  * sin((I + 1) * a),  y1, r  * cos((I + 1) * a));

			gen_rectangle_write_nodivs_intern(outf, Rectangle(P1, P2, P3, P4));
		}
	}
}

static void gen_xmas_tree3_write_intern (FILE * outf, struct Cone c)
{
	const float a = (float) ((2 * M_PI) / (float) c.slices);
	const struct Point O = Point(0, 0, 0);
	const float st = (float) c.stacks;

	for (unsigned i = 0; i < c.slices; i++) {
		const float I = (float) i;

		/* draw top mini cone */
		{
			const float R = c.rad / st;
			const float H = c.height * (st - 1) / st;

			const float xi = R * sin(I * a);
			const float zi = R * cos(I * a);

			const float xi1 = R * sin((I + 1) * a);
			const float zi1 = R * cos((I + 1) * a);

			gen_triangle_write_intern(outf, Triangle(
						Point(0,   c.height, 0),
						Point(xi,  H,        zi),
						Point(xi1, H,        zi1)));
		}

		/* draw base */
		{
			const float xi = c.rad * sin(I * a);
			const float zi = c.rad * cos(I * a);

			const float xi1 = c.rad * sin((I + 1) * a);
			const float zi1 = c.rad * cos((I + 1) * a);

			gen_triangle_write_intern(outf, Triangle(
						Point(xi,  0, zi),
						Point(0,   0, 0),
						Point(xi1, 0, zi1)));
		}

		/* draw side */
		for (unsigned j = 0; j < (c.stacks - 1); j++) {
			const float J = (float) j;

			const float y  = c.height * ((J + 1) / st);
			const float y1 = c.height * (J / st);

			const float r  = c.rad * y  / c.height;
			const float r1 = c.rad * y1 / c.height;

			const struct Point P1 = Point(r1 * sin(I       * a),  y1, r1 * cos(I       * a));
			const struct Point P2 = Point(r  * sin(I       * a),  y,  r  * cos(I       * a));
			const struct Point P3 = Point(r1 * sin((I + 1) * a),  y1, r1 * cos((I + 1) * a));
			const struct Point P4 = Point(r  * sin((I + 1) * a),  y1, r  * cos((I + 1) * a));

			gen_rectangle_write_nodivs_intern(outf, Rectangle(P1, P2, P3, P4));
		}
	}
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
static void gen_cone_write_intern (FILE * outf, struct Cone c)
{
	const float a = (float) ((2 * M_PI) / (float) c.slices);
	const struct Point O = Point(0, 0, 0);
	const float st = (float) c.stacks;

	for (unsigned i = 0; i < c.slices; i++) {
		const float I = (float) i;

		/* draw top mini cone */
		{
			const float R = c.rad / st;
			const float H = c.height * (st - 1) / st;

			const float xi = R * sin(I * a);
			const float zi = R * cos(I * a);

			const float xi1 = R * sin((I + 1) * a);
			const float zi1 = R * cos((I + 1) * a);

			gen_triangle_write_intern(outf, Triangle(
						Point(0,   c.height, 0),
						Point(xi,  H,        zi),
						Point(xi1, H,        zi1)));
		}

		/* draw base */
		{
			const float xi = c.rad * sin(I * a);
			const float zi = c.rad * cos(I * a);

			const float xi1 = c.rad * sin((I + 1) * a);
			const float zi1 = c.rad * cos((I + 1) * a);

			gen_triangle_write_intern(outf, Triangle(
						Point(xi,  0, zi),
						Point(0,   0, 0),
						Point(xi1, 0, zi1)));
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

			gen_rectangle_write_nodivs_intern(outf, Rectangle(P1, P2, P3, P4));
		}
	}
}

static void gen_cylinder_write_intern (FILE * outf, struct Cylinder c)
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
