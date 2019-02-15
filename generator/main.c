#include "generators.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int usage_square (int argc, const char ** argv)
{
    (void) argc;
    printf("\t%s square OUTFILE WIDTH DEPTH\n", *argv);
    return !0;
}

int usage_cube (int argc, const char ** argv)
{
    (void) argc;
    printf("\t%s cube OUTFILE WIDTH HEIGHT DEPTH\n", *argv);
    return !0;
}

int usage (int argc, const char ** argv)
{
    printf(
            "Usage:\n"
            "\t%s FIGURE OUTFILE <ARGS>\n"
            "\n",
            *argv);
    usage_square(argc, argv);
    usage_cube(argc, argv);
    return !0;
}

int main_square (int argc, const char ** argv)
{
    if (argc < 5)
        return usage_square(argc, argv);

    FILE * outf = fopen(argv[2], "w");

    float w = 0;
    float d = 0;

    sscanf(argv[3], "%f", &w);
    sscanf(argv[4], "%f", &d);

    gen_square_write(outf, gen_square_from_wd(w, d));

    return 0;
}

int main_cube (int argc, const char ** argv)
{
    if (argc < 6)
        return usage_cube(argc, argv);

    FILE * outf = fopen(argv[2], "w");

    float w = 0;
    float h = 0;
    float d = 0;

    sscanf(argv[3], "%f", &w);
    sscanf(argv[4], "%f", &h);
    sscanf(argv[5], "%f", &d);

    gen_cube_write(outf, gen_cube_from_whd(w, h, d));

    return 0;
}

int main (int argc, const char ** argv)
{
    if (argc < 2)
        return usage(argc, argv);

    /* generator FIGURE OUTFILE <args> */
#define cmd(fig, func) \
    (strcmp(argv[1], fig) == 0) ? func(argc, argv)

    return
        cmd("cube", main_cube):
            cmd("square", main_square):
                usage(argc, argv);
}
