#ifndef _SCENE_H
#define _SCENE_H

#include "../generator/generators.h"

#include "pugixml/pugixml.hpp"

#include <map>
#include <string>
#include <vector>

enum type {
    GT_ROTATE,
    GT_ROTATE_ANIM,
    GT_SCALE,
    GT_TRANSLATE,
    GT_TRANSLATE_ANIM,
};

struct gt {
    /**
     * GT_ROTATE: axis to rotate around
     * GT_TRANSLATE: translation offset
     * GT_SCALE: scaling scalars
     */
    struct Point p;

    /** GT_ROTATE: angle to rotate */
    float angle;

    /**
     * GT_ROTATE_ANIM: time in msecs for a full 360 rotation
     * GT_TRANSLATE_ANIM: time in msecs to execute the full animation
     */
    unsigned time;

    /** GT_TRANSLATE_ANIM: Catmull-Rom Control Points */
    std::vector<struct Point> control_points;

    /** what kind of Geometric Transformation? */
    enum type type;
};

struct group {
    std::vector<struct gt>     gt;
    std::vector<std::string>   models;
    std::vector<struct group*> subgroups;
};

struct model {
    /** VBO ID */
    GLuint id;

    /** Vertex count */
    size_t length;
};

struct scene {
    std::vector<struct group*> groups;
    std::map<std::string, struct model> models;
};

bool sc_load_file (const char * path, struct scene * scene);
void sc_draw      (struct scene * scene, unsigned int elapsed, bool draw_curves);

#endif /* _SCENE_H */
