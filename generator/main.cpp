#include "generators.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int usage_rectangle (int argc, const char ** argv)
{
    (void) argc;
    printf("\t%s rectangle OUTFILE WIDTH DEPTH\n", *argv);
    return !0;
}

int usage_box (int argc, const char ** argv)
{
    (void) argc;
    printf("\t%s box OUTFILE WIDTH HEIGHT DEPTH\n", *argv);
    return !0;
}

int usage_sphere (int argc, const char ** argv)
{
    (void) argc;
    printf("\t%s sphere OUTFILE RADIUS SLICES STACKS\n", *argv);
    return !0;
}

int usage (int argc, const char ** argv)
{
    printf(
            "Usage:\n"
            "\t%s FIGURE OUTFILE <ARGS>\n"
            "\n",
            *argv);
    usage_rectangle(argc, argv);
    usage_box(argc, argv);
    usage_sphere(argc, argv);
    return !0;
}

int main_rectangle (FILE * outf, int argc, const char ** argv)
{
    if (argc < 5)
        return usage_rectangle(argc, argv);

    float w = 0;
    float d = 0;

    sscanf(argv[3], "%f", &w);
    sscanf(argv[4], "%f", &d);

    gen_rectangle_write(outf, gen_rectangle_from_wd(w, d));

    return 0;
}

int main_box (FILE * outf, int argc, const char ** argv)
{
    if (argc < 6)
        return usage_box(argc, argv);

    float w = 0;
    float h = 0;
    float d = 0;

    sscanf(argv[3], "%f", &w);
    sscanf(argv[4], "%f", &h);
    sscanf(argv[5], "%f", &d);

    gen_box_write(outf, gen_box_from_whd(w, h, d));

    return 0;
}

int main_sphere (FILE * outf, int argc, const char ** argv)
{
    if (argc < 6)
        return usage_sphere(argc, argv);

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
        return usage(argc, argv);

    FILE * outf = (argc > 2) ?
        fopen(argv[2], "w"):
        NULL;

    /* generator FIGURE OUTFILE <args> */
#define cmd(fig, func) \
    (strcmp(argv[1], fig) == 0) ? func(outf, argc, argv)

    return
        cmd("box", main_box):
        cmd("rectangle", main_rectangle):
        cmd("sphere", main_sphere):
        usage(argc, argv);
}
