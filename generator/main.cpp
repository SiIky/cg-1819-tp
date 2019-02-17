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

int usage (int argc, const char ** argv)
{
    printf(
            "Usage:\n"
            "\t%s FIGURE OUTFILE <ARGS>\n"
            "\n",
            *argv);
    usage_rectangle(argc, argv);
    usage_box(argc, argv);
    return !0;
}

int main_rectangle (int argc, const char ** argv)
{
    if (argc < 5)
        return usage_rectangle(argc, argv);

    FILE * outf = fopen(argv[2], "w");

    float w = 0;
    float d = 0;

    sscanf(argv[3], "%f", &w);
    sscanf(argv[4], "%f", &d);

    gen_rectangle_write(outf, gen_rectangle_from_wd(w, d));

    return 0;
}

int main_box (int argc, const char ** argv)
{
    if (argc < 6)
        return usage_box(argc, argv);

    FILE * outf = fopen(argv[2], "w");

    float w = 0;
    float h = 0;
    float d = 0;

    sscanf(argv[3], "%f", &w);
    sscanf(argv[4], "%f", &h);
    sscanf(argv[5], "%f", &d);

    gen_box_write(outf, gen_box_from_whd(w, h, d));

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
        cmd("box", main_box):
            cmd("rectangle", main_rectangle):
                usage(argc, argv);
}
