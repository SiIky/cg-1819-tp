/**
 * Graphical Primitive Generator (Main Module)
 * Last Updated: 22-02-2019
 */

#include "generators.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/**
 * @brief Display the user information on how to build a rectangle.
 * @param argv - Programme name (function will only be called if argv < 2).
 * @return 0, for success.
 */
int usage_rectangle (const char ** argv) {
    printf("\t%s rectangle OUTFILE WIDTH DEPTH [NDIVS]\n", *argv);
    return !0;
}

/**
 * @brief Display the user information on how to build a box.
 * @param argv - Programme name (function will only be called if argv < 2).
 * @return 0, for success.
 */
int rusage_box (const char ** argv)
{
    printf("\t%s box OUTFILE WIDTH HEIGHT DEPTH [NDIVS]\n", *argv);
    return !0;
}

/**
 * @brief Display the user information on how to build a cone.
 * @param argv - Programme name (function will only be called if argv < 2).
 * @return 0, for success.
 */
int usage_cone (const char ** argv)
{
    printf("\t%s cone OUTFILE RADIUS HEIGHT SLICES\n", *argv);
    return !0;
}

/**
 * @brief Display the user information on how to build a cylinder.
 * @param argv - Programme name (function will only be called if argv < 2).
 * @return 0, for success.
 */
int usage_cylinder (const char ** argv)
{
    printf("\t%s cylinder OUTFILE RADIUS HEIGHT SLICES STACKS\n", *argv);
    return !0;
}

/**
 * @brief Display the user information on how to build a sphere.
 * @param argv - Programme name (function will only be called if argv < 2).
 * @return 0, for success.
 */
int usage_sphere (const char ** argv)
{
    printf("\t%s sphere OUTFILE RADIUS SLICES STACKS\n", *argv);
    return !0;
}

/** 
 * @brief Displays the user information on how to run the programme.
 * @param argv - Programme name (function will only be called if argv < 2).
 * @return 0, for success.
 */
int usage (const char ** argv)
{
    printf(
            "Usage:\n"
            "\t%s FIGURE OUTFILE <ARGS>\n"
            "\n",
            *argv);
    usage_rectangle(argv);
    usage_box(argv);
    usage_cone(argv);
    usage_cylinder(argv);
    usage_sphere(argv);
    return !0;
}

/**
 * @brief Main function for generating a rectangle. 
 * @param argc - Number of given arguments.
 * @param argv - Argument values.
 * @return 0, for success.
 */
int main_rectangle (FILE * outf, int argc, const char ** argv)
{
    if (argc < 5)
        return usage_rectangle(argv);

    float w = 0;
    float d = 0;

    sscanf(argv[3], "%f", &w);
    sscanf(argv[4], "%f", &d);

    unsigned ndivs = 0;
    if (argc > 5)
        sscanf(argv[5], "%u", &ndivs);

    gen_rectangle_write(outf, gen_rectangle_from_wd(w, d), ndivs);

    return 0;
}

/**
 * @brief Main function for generating a box. 
 * @param argc - Number of given arguments.
 * @param argv - Argument values.
 * @return 0, for success.
 */
int main_box (FILE * outf, int argc, const char ** argv)
{
    if (argc < 6)
        return usage_box(argv);

    float w = 0;
    float h = 0;
    float d = 0;

    sscanf(argv[3], "%f", &w);
    sscanf(argv[4], "%f", &h);
    sscanf(argv[5], "%f", &d);

    unsigned ndivs = 0;
    if (argc > 6)
        sscanf(argv[6], "%u", &ndivs);

    gen_box_write(outf, gen_box_from_whd(w, h, d), ndivs);

    return 0;
}

/**
 * @brief Main function for generating a cone. 
 * @param argc - Number of given arguments.
 * @param argv - Argument values.
 * @return 0, for success.
 */
int main_cone (FILE * outf, int argc, const char ** argv)
{
    if (argc < 6)
        return usage_cone(argv);

    struct Cone c = Cone(0, 0, 0);
    sscanf(argv[3], "%f", &c.rad);
    sscanf(argv[4], "%f", &c.height);
    sscanf(argv[5], "%u", &c.slices);

    gen_cone_write(outf, c);
    return 0;
}

/**
 * @brief Main function for generating a cylinder. 
 * @param argc - Number of given arguments.
 * @param argv - Argument values.
 * @return 0, for success.
 */
int main_cylinder (FILE * outf, int argc, const char ** argv)
{
    if (argc < 7)
        return usage_cylinder(argv);

    struct Cylinder c = Cylinder(0, 0, 0, 0);
    sscanf(argv[3], "%f", &c.rad);
    sscanf(argv[4], "%f", &c.height);
    sscanf(argv[5], "%u", &c.slices);
    sscanf(argv[6], "%u", &c.stacks);

    gen_cylinder_write(outf, c);
    return 0;
}

/**
 * @brief Main function for generating a sphere. 
 * @param argc - Number of given arguments.
 * @param argv - Argument values.
 * @return 0, for success.
 */
int main_sphere (FILE * outf, int argc, const char ** argv)
{
    if (argc < 6)
        return usage_sphere(argv);

    struct Sphere sph = Sphere(0, 0, 0);

    sscanf(argv[3], "%f", &sph.rad);
    sscanf(argv[4], "%u", &sph.slices);
    sscanf(argv[5], "%u", &sph.stacks);

    gen_sphere_write(outf, sph);

    return 0;
}

int main (int argc, const char ** argv)
{
    if (argc < 2)
        return usage(argv);

    FILE * outf = (argc > 2) ?
        fopen(argv[2], "w"):
        NULL;

    /* generator FIGURE OUTFILE <args> */
#define cmd(fig, func) \
    (strcmp(argv[1], fig) == 0) ? func(outf, argc, argv)

    return
        cmd("rectangle", main_rectangle):
        cmd("box", main_box):
        cmd("cone", main_cone):
        cmd("cylinder", main_cylinder):
        cmd("sphere", main_sphere):
        usage(argv);
}
